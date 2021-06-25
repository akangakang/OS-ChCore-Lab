BUILD_DIR := ./build
ifndef QEMU
QEMU := qemu-system-aarch64
endif

LAB := 4
# try to generate a unique GDB port
GDBPORT	:= 1234
QEMUOPTS = -machine raspi3 -serial null -serial mon:stdio -m size=1G -kernel $(BUILD_DIR)/kernel.img -gdb tcp::1234
IMAGES = $(BUILD_DIR)/kernel.img

all: user build

gdb:
	gdb-multiarch -n -x .gdbinit

build: FORCE
	./scripts/docker_build.sh $(bin)

user: FORCE
	./scripts/docker_build_user.sh

qemu: $(IMAGES) 
	$(QEMU) $(QEMUOPTS)

qemu-gdb: $(IMAGES)
	@echo "***"
	@echo "*** make qemu-gdb'." 1>&2
	@echo "***"
	$(QEMU) -nographic $(QEMUOPTS) -S


gdbport:
	@echo $(GDBPORT)


docker: FORCE	
	./scripts/run_docker.sh

grade: build FORCE
	@echo "make grade"
	@echo "LAB"$(LAB)": test >>>>>>>>>>>>>>>>>"
ifeq ($(LAB), 2)
	./scripts/run_mm_test.sh
endif
	./scripts/grade-lab$(LAB)

.PHONY: clean
clean:
	@rm -rf build
	@rm -rf chcore.out

prep-%:
	@echo "*** Now building application $*"
	./scripts/docker_build.sh $*
	./scripts/create_gdbinit.sh $*

run-%: prep-%
	@echo "*** Now starting qemu"
	$(QEMU) $(QEMUOPTS)

run-%-gdb: prep-%
	@echo "*** Now starting qemu-gdb"
	$(QEMU) $(QEMUOPTS) -S

.PHONY: FORCE
FORCE:
