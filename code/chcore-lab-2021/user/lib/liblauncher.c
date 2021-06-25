#include <lib/defs.h>
#include <lib/syscall.h>
#include <lib/errno.h>
#include <lib/launcher.h>
#include <lib/ipc.h>
#include <lib/bug.h>
#include <lib/print.h>
#include <lib/string.h>
#include <lib/fs_defs.h>

#define VMR_READ  (1 << 0)
#define VMR_WRITE (1 << 1)
#define VMR_EXEC  (1 << 2)

#define PFLAGS2VMRFLAGS(PF) \
	(((PF)&PF_X ? VMR_EXEC : 0) | ((PF)&PF_W ? VMR_WRITE : 0) | \
	 ((PF)&PF_R ? VMR_READ : 0))

#define OFFSET_MASK 0xfff

int parse_elf_from_binary(const char *binary, struct user_elf *user_elf)
{
	int ret;
	struct elf_file elf;
	size_t seg_sz, seg_map_sz;
	u64 p_vaddr;
	int i;
	int j;
	u64 tmp_vaddr = 0xc00000;

	elf_parse_file(binary, &elf);

	/* init pmo, -1 indicates that this pmo is not used
	 *
	 * Currently, an elf file can have 2 PT_LOAD segment at most
	 */
	for (i = 0; i < 2; ++i)
		user_elf->user_elf_seg[i].elf_pmo = -1;

	for (i = 0, j = 0; i < elf.header.e_phnum; ++i) {
		if (elf.p_headers[i].p_type != PT_LOAD)
			continue;

		seg_sz = elf.p_headers[i].p_memsz;
		p_vaddr = elf.p_headers[i].p_vaddr;
		BUG_ON(elf.p_headers[i].p_filesz > seg_sz);
		seg_map_sz = ROUND_UP(seg_sz + p_vaddr, PAGE_SIZE) -
		    ROUND_DOWN(p_vaddr, PAGE_SIZE);

		user_elf->user_elf_seg[j].elf_pmo =
		    usys_create_pmo(seg_map_sz, PMO_DATA);
		BUG_ON(user_elf->user_elf_seg[j].elf_pmo < 0);

		ret = usys_map_pmo(SELF_CAP,
				   user_elf->user_elf_seg[j].elf_pmo,
				   tmp_vaddr, VM_READ | VM_WRITE);
		BUG_ON(ret < 0);

		memset((void *)tmp_vaddr, 0, seg_map_sz);
		/*
		 * OFFSET_MASK is for calculating the final offset for loading
		 * different segments from ELF.
		 * ELF segment can specify not aligned address.
		 *
		 */
		memcpy((void *)tmp_vaddr +
		       (elf.p_headers[i].p_vaddr & OFFSET_MASK),
		       (void *)(binary +
				elf.p_headers[i].p_offset),
		       elf.p_headers[i].p_filesz);

		user_elf->user_elf_seg[j].seg_sz = seg_sz;
		user_elf->user_elf_seg[j].p_vaddr = p_vaddr;
		user_elf->user_elf_seg[j].flags =
		    PFLAGS2VMRFLAGS(elf.p_headers[i].p_flags);
		usys_unmap_pmo(SELF_CAP,
			       user_elf->user_elf_seg[j].elf_pmo, tmp_vaddr);

		j++;
	}

	user_elf->elf_meta.phdr_addr = elf.p_headers[0].p_vaddr +
	    elf.header.e_phoff;
	user_elf->elf_meta.phentsize = elf.header.e_phentsize;
	user_elf->elf_meta.phnum = elf.header.e_phnum;
	user_elf->elf_meta.flags = elf.header.e_flags;
	user_elf->elf_meta.entry = elf.header.e_entry;

	return 0;
}

void *single_file_handler(const void *start, size_t size, void *data)
{
	struct user_elf *user_elf = data;
	void *ret;

	ret = (void *)(s64) parse_elf_from_binary(start, user_elf);
	return ret;
}

int readelf_from_kernel_cpio(const char *filename, struct user_elf *user_elf)
{
	int ret;

	strcpy(user_elf->path, filename);
	ret = (int)(s64) cpio_extract_single((void *)CPIO_BIN, filename,
					     single_file_handler, user_elf);

	return ret;
}

ipc_struct_t *tmpfs_ipc_struct;
static int fs_read(const char *path, int *tmpfs_read_pmo_cap)
{
	ipc_msg_t *ipc_msg;
	int ret;
	struct fs_request fr;

	/* IPC send cap */
	ipc_msg = ipc_create_msg(tmpfs_ipc_struct,
				 sizeof(struct fs_request), 1);
	fr.req = FS_REQ_GET_SIZE;
	strcpy((void *)fr.path, path);
	ipc_set_msg_data(ipc_msg, (char *)&fr, 0, sizeof(struct fs_request));
	ret = ipc_call(tmpfs_ipc_struct, ipc_msg);

	*tmpfs_read_pmo_cap = usys_create_pmo(ret, PMO_DATA);

	fr.req = FS_REQ_READ;
	strcpy((void *)fr.path, path);
	fr.offset = 0;
	fr.buff = (char *)TMPFS_READ_BUF_VADDR;
	fr.count = ret;
	fr.req = FS_REQ_READ;
	ipc_set_msg_cap(ipc_msg, 0, *tmpfs_read_pmo_cap);
	ipc_set_msg_data(ipc_msg, (char *)&fr, 0, sizeof(struct fs_request));
	ret = ipc_call(tmpfs_ipc_struct, ipc_msg);

	ipc_destroy_msg(ipc_msg);
	return ret;
}

int readelf_from_fs(const char *pathbuf, struct user_elf *user_elf)
{
	int ret;
	int tmpfs_read_pmo_cap;

	ret = fs_read(pathbuf, &tmpfs_read_pmo_cap);
	if (ret < 0) {
		return ret;
	}

	ret = usys_map_pmo(SELF_CAP,
			   tmpfs_read_pmo_cap,
			   TMPFS_READ_BUF_VADDR, VM_READ | VM_WRITE);
	BUG_ON(ret < 0);

	strcpy(user_elf->path, pathbuf);
	ret = parse_elf_from_binary((const char *)TMPFS_READ_BUF_VADDR,
				    user_elf);

	usys_unmap_pmo(SELF_CAP, tmpfs_read_pmo_cap, TMPFS_READ_BUF_VADDR);

	return ret;
}
