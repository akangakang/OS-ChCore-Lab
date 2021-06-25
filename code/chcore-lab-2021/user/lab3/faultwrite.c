// buggy program - faults with a write to location zero

int main(int argc, char *argv[], char *envp[])
{
	*(unsigned *)1 = 0;
	return 0;
}
