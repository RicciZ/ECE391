#include "tests.h"
#include "x86_desc.h"
#include "library/lib.h"
#include "interrupt/idt_init.h"
#include "paging.h"
#include "terminal.h"
#include "interrupt/rtc.h"
#include "filesys.h"
#include "process.h"
#include "interrupt/sys_call.h"
#include "speaker.h"
#include "interrupt/rtc.h"
#include "interrupt/sb16.h"
#include "library/dynamic_allocation.h"

#define PASS 1
#define FAIL 0


// a macro to choose which checkpoint tests to compile
#define CHECKPOINT 6



/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


#if (CHECKPOINT == 1)
/* Checkpoint 1 tests */


/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}



/* divide_by_zero_test
 *
 * Test exception divide by zero.
 * Inputs: None
 * Outputs: None
 * Side Effects: freeze kernel.
 * Coverage: Exception handler.
 * Files: interrupt.c/S
 */
void divide_by_zero_test(){
	TEST_HEADER;
	int a;
	int b = 1;
	int c = 0;
	a = b/c;
}



/* exception_sim
 * 
 * Simulate an exception.
 * Inputs: None
 * Outputs: Exception message.
 * Side Effects: None
 * Coverage: Exception
 * Files: interrupt.c/S
 */
static inline void exception_sim(){
	asm volatile("int $6");
}



/* syscall_sim
 * 
 * Simulate a system call.
 * Inputs: None
 * Outputs: System call message.
 * Side Effects: None
 * Coverage: System call.
 * Files: interrupt.c/S
 */
static inline void syscall_sim(){
	asm volatile("int $0x80");
	asm volatile("int $0x80");
	asm volatile("int $0x80");
	asm volatile("int $0x80");
}



/* deference_null_test
 * 
 * try deferencing the null pointer
 * Inputs: None
 * Outputs: 
 * Side Effects: should raise an exception
 * Coverage: paging and exception handler
 * Files: paging.c and interrupt.c
 */
int dereference_null_test(){
	TEST_HEADER;
	printf("--This test dereferences a NULL pointer, which should raise an exception.\n");
	int* ptr = (int*)NULL;
	int var;
	var = *(ptr);
	return FAIL;
}



/* paging_video_memory_test
 * 
 * try deferencing the valid addresses of the video memory
 * Inputs: None
 * Outputs: PASS or Exception
 * Side Effects: should return PASS
 * Coverage: paging
 * Files: paging.c
 */
int paging_video_memory_test(){
	TEST_HEADER;
	printf("--This test dereferences the start of the video memory.\n");
	uint8_t* ptr1 = (uint8_t*)VIDEO_MEM_ADDR;
	int var1;
	var1 = *(ptr1);

	printf("--This test dereferences a random address of the video memory.\n");
	uint8_t* ptr2 = (uint8_t*)(VIDEO_MEM_ADDR + 16);
	int var2;
	var2 = *(ptr2);

	printf("--This test dereferences the end address of the video memory.\n");
	uint8_t* ptr3 = (uint8_t*)(VIDEO_MEM_END);
	int var3;
	var3 = *(ptr3);
	return PASS;
}



/* paging_video_boundary_up_test
 * 
 * try deferencing the boundary of the video memory (outside the video memory)
 * Inputs: None
 * Outputs: PASS or Exception
 * Side Effects: should raise an exception
 * Coverage: paging
 * Files: paging.c and interrupt.c
 */
int paging_video_boundary_up_test(){
	TEST_HEADER;
	printf("--This test dereferences the upper boundary of the video memory, which should raise an exception.\n");
	uint8_t* ptr = (uint8_t*)(VIDEO_MEM_ADDR - 1);
	int var;
	var = *(ptr);
	return PASS;
}



/* paging_video_boundary_down_test
 * 
 * try deferencing the boundary of the video memory (outside the video memory)
 * Inputs: None
 * Outputs: PASS or Exception
 * Side Effects: should raise an exception
 * Coverage: paging
 * Files: paging.c and interrupt.c
 */
int paging_video_boundary_down_test(){
	TEST_HEADER;
	printf("--This test dereferences the lower boundary of the video memory, which should raise an exception.\n");
	uint8_t* ptr = (uint8_t*)(VIDEO_MEM_END + 1);
	int var;
	var = *(ptr);
	return PASS;
}



/* paging_not_present_test1
 * 
 * try deferencing a non-present address
 * Inputs: None
 * Outputs: 
 * Side Effects: should raise an exception
 * Coverage: paging and exception handler
 * Files: paging.c and interrupt.c
 */
int paging_not_present_test1(){
	TEST_HEADER;
	printf("--This test dereferences a non-present address, which should raise an exception.\n");
	int* ptr = (int*)(0x800000 + 4);	// for checkpoint 1, any memory larger than 8 MB is not present
	int var;
	var = *(ptr);
	return FAIL;
}



/* paging_not_present_test2
 * 
 * try deferencing a non-present address
 * Inputs: None
 * Outputs: 
 * Side Effects: should raise an exception
 * Coverage: paging and exception handler
 * Files: paging.c and interrupt.c
 */
int paging_not_present_test2(){
	TEST_HEADER;
	printf("--This test dereferences a non-present address, which should raise an exception.\n");
	int* ptr = (int*)(400);	// for checkpoint 1, any memory address less than the video memory is not present
	int var;
	var = *(ptr);
	return FAIL;
}



/* paging_kernel_memory_test
 * 
 * try deferencing the valid addresses of the kernel memory
 * Inputs: None
 * Outputs: PASS or Exception
 * Side Effects: should return PASS
 * Coverage: paging
 * Files: paging.c
 */
int paging_kernel_memory_test(){
	TEST_HEADER;
	printf("--This test dereferences the start of the kernel memory.\n");
	uint8_t* ptr1 = (uint8_t*)KERNEL_MEM_ADDR;
	int var1;
	var1 = *(ptr1);

	printf("--This test dereferences a random address of the kernel memory.\n");
	uint8_t* ptr2 = (uint8_t*)(KERNEL_MEM_ADDR + 36);
	int var2;
	var2 = *(ptr2);

	printf("--This test dereferences the end address of the kernel memory.\n");
	uint8_t* ptr3 = (uint8_t*)(KERNEL_MEM_END);
	int var3;
	var3 = *(ptr3);
	return PASS;
}



/* paging_kernel_boundary_up_test
 * 
 * try deferencing the boundary of the kernel memory (outside the video memory)
 * Inputs: None
 * Outputs: PASS or Exception
 * Side Effects: should raise an exception
 * Coverage: paging
 * Files: paging.c and interrupt.c
 */
int paging_kernel_boundary_up_test(){
	TEST_HEADER;
	printf("--This test dereferences the upper boundary of the kernel memory, which should raise an exception.\n");
	uint8_t* ptr = (uint8_t*)(KERNEL_MEM_ADDR - 1);
	int var;
	var = *(ptr);
	return PASS;
}



/* paging_kernel_boundary_down_test
 * 
 * try deferencing the boundary of the kernel memory (outside the video memory)
 * Inputs: None
 * Outputs: PASS or Exception
 * Side Effects: should raise an exception
 * Coverage: paging
 * Files: paging.c and interrupt.c
 */
int paging_kernel_boundary_down_test(){
	TEST_HEADER;
	printf("--This test dereferences the lower boundary of the kernel memory, which should raise an exception.\n");
	uint8_t* ptr = (uint8_t*)(KERNEL_MEM_END + 1);
	int var;
	var = *(ptr);
	return PASS;
}



// checkpoint_1 launch test
void launch_tests(){
	
	TEST_OUTPUT("idt_test", idt_test());
	// exception_sim();
	syscall_sim();
	// divide_by_zero_test();
	TEST_OUTPUT("paging_video_memory_test", paging_video_memory_test());
	TEST_OUTPUT("paging_kernel_memory_test", paging_kernel_memory_test());
	// TEST_OUTPUT("paging_video_boundary_up_test", paging_video_boundary_up_test());
	// TEST_OUTPUT("paging_video_boundary_up_test", paging_video_boundary_up_test());
	// TEST_OUTPUT("paging_kernel_boundary_up_test", paging_kernel_boundary_up_test());
	// TEST_OUTPUT("paging_kernel_boundary_down_test" , paging_kernel_boundary_down_test());
	// TEST_OUTPUT("paging_not_present_test1", paging_not_present_test1());
	// TEST_OUTPUT("paging_not_present_test2", paging_not_present_test2());
	// TEST_OUTPUT("dereference_null_test", dereference_null_test());
	clear();
}

#endif



#if (CHECKPOINT == 2)
/* Checkpoint 2 tests */


/* rtc_ocrw_test
 * 
 * Test RTC open, close, read, write.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: RTC open, close, read, write
 * Files: rtc.c/h
 */
int rtc_ocrw_test(){
	TEST_HEADER;

	int32_t freq;
	int32_t fakeinput;
	int i;
	rtc_open(NULL);
	for (i = 0; i < 4; i++){
		rtc_read(fakeinput,NULL,fakeinput);
		printf("initial");
	}
	for (freq = 1; freq < 1200; freq++){
		if (rtc_write(fakeinput,(void*) &freq,fakeinput)) continue;
		for (i = 0; i < freq; i++){
			rtc_read(fakeinput,NULL,fakeinput);
			printf("%d", freq);
		}
	}
	printf("\n");
	rtc_close(fakeinput);
	return PASS;
}



/* keyboard_terminal_test
 * 
 * Test terminal open, close, read, write.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: terminal (keyboard) open, close, read, write
 * Files: keyboard.c/h  /  terminal.c/h
 */
int keyboard_terminal_test(){
	TEST_HEADER;

	// though the keyboard buffer contains only 128 bytes, we use a 256-byte buffer to store the contents
	uint8_t terminal_buffer [256];	
	uint32_t num_read;
	uint32_t fd = 0;	// fake file descriptor
	uint32_t i;			// loop index
	uint32_t break_flag = 1;
	terminal_open(NULL);
	while (1){
		break_flag = 1;
		num_read  = terminal_read(fd, terminal_buffer, 128) - 1; // - 1 is due to we also read a '\0'
		// check whether the user asks to quit
		for (i = 0; *(uint8_t*)(terminal_buffer + i) != '\0'; ++i){
			switch (i){
				case 0:
					if (*(uint8_t*)(terminal_buffer + i) != 'q') break_flag = 0;
					break;
				case 1:
					if (*(uint8_t*)(terminal_buffer + i) != 'u') break_flag = 0;
					break;
				case 2:
					if (*(uint8_t*)(terminal_buffer + i) != 'i') break_flag = 0;
					break;
				case 3:
					if (*(uint8_t*)(terminal_buffer + i) != 't') break_flag = 0;
					break;
				case 4:
					if (*(uint8_t*)(terminal_buffer + i) != '\n') break_flag = 0;
					break;
				default: ;
			}
		}
		if (break_flag == 1) break;
		terminal_write(fd, terminal_buffer, num_read);
	}
	terminal_close(fd); 
	extern uint32_t flag_function;
	flag_function = 0;
	return PASS;
}



/* file_system_test
 * 
 * Test file system open, close, read, write.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: file open, close, read, write
 * Files: filesys.c/h
 */
int file_system_test(){
	TEST_HEADER;
	
	//test read a file
	uint8_t fname[] = "frame0.txt";
	if (file_open(fname) == -1){
		printf("file not found.\n");
		return PASS;
	}

	//buffer used to test
	uint8_t buffer [36864];
	extern dentry_t dummy_fd;
	extern inode_t* inode_start;
	uint32_t num_read = file_read(0, buffer, (inode_start + dummy_fd.inode)->size);
	printf("Number of bytes read: %d\n", num_read);
	terminal_write(0, buffer, num_read);
	
	file_close(0);
	rtc_open(NULL);
	int i;
	int freq = 2;
	//2Hz 1s
	for (i = 0; i < freq; i++){
		rtc_read(0,NULL,0);
	}
	file_open(fname);
	uint32_t read_len = 10;
	uint32_t total_len = 0;
	printf("\n\nseparate read test: every %d bytes\n",read_len);

	//separate read test every 10 bytes 1s
	while (read_len == 10)
	{
		read_len = file_read(0, buffer, read_len);
		terminal_write(0, buffer, read_len);
		total_len += read_len;
		for (i = 0; i < freq; i++){
			rtc_read(0,NULL,0);
		}
	}
	rtc_close(NULL);
	printf("Total Number of bytes read: %d\n", total_len);

	//test write a file
	printf("\nTry to write a file.\n");
	if (file_write(0, buffer, 128) != -1){
		printf("file write failed.\n");
		return FAIL;
	}
	printf("Writing forbidden.\n\n");

	//test close a file
	printf("Close the file.\n\n");
	file_close(0);

	printf("Try to read the file after closing it.\n");
	printf("File read return value: %d.\n", file_read(0, buffer, (inode_start + dummy_fd.inode)->size));

	return PASS;
}



/* dir_read_test
 * 
 * Test directory read.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: filesys.c/h
 */
int dir_read_test(){
	TEST_HEADER;

	int32_t i;
	uint8_t dir_name[] = ".";
	if (dir_open(dir_name)) return FAIL;
	char buf[MAX_FILENAME_LEN + 1];
	extern boot_blk_t* boot_blk_ptr;
	for (i = 0; i < boot_blk_ptr->num_dentries; i++){
		dir_read(0,buf,0);
		printf("%s\n", buf);
	}
	printf("\nClose the file.\n");
	dir_close(0);

	printf("Try to read the file after closing it.\n");
	printf("Directory read return value: %d.\n", dir_read(0,buf,0));
	return PASS;
}



/* integrated_test_CP2
 * 
 * Test whole checkpoint 2 related files
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: checkpoint 2
 */
int integrated_test_cp2(){
	TEST_HEADER;

	TEST_OUTPUT("rtc open close read write test", rtc_ocrw_test());

	TEST_OUTPUT("directory read test", dir_read_test());

	uint8_t buffer [36864];		// we want the buffer to be large enough to hold everything!
	uint32_t num_read;
	uint32_t fd = 0;	// fake file descriptor
	uint32_t i;			// loop index
	uint32_t break_flag = 1;
	terminal_open(NULL);
	while (1){
		break_flag = 1;
		printf("\nType in the file name, or type 'quit' to kill the program.\n");
		num_read  = terminal_read(fd, buffer, 128) - 1; // - 1 is due to we also read a '\0'
		// check whether the user asks to quit
		for (i = 0; *(uint8_t*)(buffer + i) != '\0'; ++i){
			switch (i){
				case 0:
					if (*(uint8_t*)(buffer + i) != 'q') break_flag = 0;
					break;
				case 1:
					if (*(uint8_t*)(buffer + i) != 'u') break_flag = 0;
					break;
				case 2:
					if (*(uint8_t*)(buffer + i) != 'i') break_flag = 0;
					break;
				case 3:
					if (*(uint8_t*)(buffer + i) != 't') break_flag = 0;
					break;
				case 4:
					if (*(uint8_t*)(buffer + i) != '\n') break_flag = 0;
					break;
				default: ;
			}
		}
		if (break_flag == 1) break;
		// preprocess the file name, in particular, get rid of '\n'
		for (i = 0; i < 36864; ++i){
			if (buffer[i] == '\n'){
				buffer[i] = '\0';
				break;
			}
		}
		if (file_open(buffer) == -1){
			printf("file not found.\n");
			continue;
		}

		extern dentry_t dummy_fd;
		extern inode_t* inode_start;
		num_read = file_read(fd, buffer, (inode_start + dummy_fd.inode)->size);
		printf("Number of bytes read: %d\n", num_read);
		terminal_write(fd, buffer, num_read);

		printf("\n\nTry to write a file.\n");
		if (file_write(fd, buffer, 128) != -1){
			printf("file write failed.\n");
			return FAIL;
		}
		printf("Writing forbidden.\n\n");

		printf("Close the file.\n\n");
		file_close(fd);

		printf("Try to read the file after closing it.\n");
		printf("File read return value: %d.\n", file_read(0, buffer, (inode_start + dummy_fd.inode)->size));
	}

	terminal_close(fd); 
	extern uint32_t flag_function;
	flag_function = 0;
	return PASS;
}

void launch_tests(){
	// TEST_OUTPUT("rtc open close read write test", rtc_ocrw_test());
	// TEST_OUTPUT("keyboard_terminal_test", keyboard_terminal_test());
	//  TEST_OUTPUT("file_system_test", file_system_test());
	// TEST_OUTPUT("directory read test", dir_read_test());
	TEST_OUTPUT("integrated_test_cp2", integrated_test_cp2());
}
#endif



#if (CHECKPOINT == 3)

/* dir_read_test
 * 
 * Test directory read.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: filesys.c/h
 */
void dir_read_test_s(){
	int32_t i;
	int32_t len;
	uint8_t dir_name[] = ".";
	int32_t fd = open(dir_name);
	if (fd == -1) printf("dir open failed\n");
	char buf[MAX_FILENAME_LEN + 1];
	extern boot_blk_t* boot_blk_ptr;
	for (i = 0; i < boot_blk_ptr->num_dentries; i++){
		len = read(fd,buf,0);
		write(1,buf,len);
		printf("\n");
	}
	printf("\nClose the dir file.\n");
	if (close(fd) == -1) printf("close dir failed\n");

	printf("Try to read the file after closing it.\n");
	printf("Directory read return value: %d.\n", read(fd,buf,0));
}

/*test_orwc
 * 
 * system call function test
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: sys_call.c/h, process.c/h
 */
 int test_orwc(){
	TEST_HEADER;

	int i,j;
	//temp buffer for test
	uint8_t* buf = (uint8_t*) 0x500000;
	uint32_t len,counter;

	if (init_fd(get_PCB(0)->fd_array) == -1) printf("initial fd failed\n");

	printf("test dir read\n");
	dir_read_test_s();

	//several tests for system call
	for (i = 0; i < 3; i++){
		printf("Input a file name to open. Three chance.\n");
		len = read(0,(void*)buf,36864);
		if (len == -1) printf("read failed\n");
		for (j = 0; j < 36864; ++j){
			if (buf[j] == '\n'){
				buf[j] = '\0';
				break;
			}
		}
		int32_t fd = open((uint8_t*)buf);
		printf("Open Test:\n");
		if (fd == -1) printf("open failed\n");
		else printf("open success\n");
		printf("Read Test:\n");
		len = read(fd,(void*)buf,36864);
		if (len == -1) printf("read failed\n");
		else printf("read success\n");
		printf("Write Test:\n");
		if (write(1,(void*)buf,len) == -1) printf("\nwrite failed\n");
		else printf("\nwrite success\n");
		printf("Close Test:\n");
		if (close(fd) == -1) printf("close failed\n"); 
		else printf("close success\n");
		if (read(fd,(void*)buf,36864) == -1) printf("file closed, cannot read\n");
	}

	printf("File Descriptor Test. Up to 6 files can be opened.\n");
	printf("You need to try 10 times to quit.\n");
	//File Descriptor Test. Up to 6 files can be opened
	for (i = 0; i < 10; i++){
		len = read(0,(void*)buf,36864);;
		if (len == -1) printf("read failed\n");
		for (j = 0; j < 36864; ++j){
			if (buf[j] == '\n'){
				buf[j] = '\0';
				break;
			}
		}
		int32_t fd = open((uint8_t*)buf);
		if (fd == -1) printf("open failed\n");
		else counter++;
		len = read(fd,(void*)buf,36864);
		if (len == -1) printf("read failed\n");
		printf("fd idx: %d, opening: %d\n",fd,counter);
	}

	//stdin, stdout test
	printf("\nstdin stdout function test.\n");
	if (close(0) == -1) printf("stdin cannot be closed\n");
	if (close(1) == -1) printf("stdout cannot be closed\n");

	if (read(1,buf,100) == -1) printf("stdout is write only\n");
	if (write(0,buf,100) == -1) printf("stdin is read only\n");

	return PASS;
}

void launch_tests(){
	TEST_OUTPUT("open read write close test", test_orwc());
}
#endif



#if (CHECKPOINT == 6) // Extra Credit

int random_test1(){
	TEST_HEADER;

	int num = 0;
	int i = 0;
	while (i < 5) {
		printf("press r to generate a random number from 0 to 32767\n");
		while (display_terminal->keyboard_buf[display_terminal->read_count-1] != 'r');
		clear_keyboard_buffer();
		display_terminal->read_count = 0;
		printf("\n");
		num = rand();
		printf("%d: %d\n",i,num);
		i++;
		if (i == 5) {
			printf("press a to try another 5 random numbers, press others to quit\n");
			while (display_terminal->read_count < 1);
			if (display_terminal->keyboard_buf[0] == 'a') i = 0;
			clear_keyboard_buffer();
			display_terminal->read_count = 0;
		}
	}
	printf("\n");

	return PASS;
}

int random_test2(){
	TEST_HEADER;

	int i;
	int num = 0;
	int zero = 0;
	int one = 0;
	int two = 0;
	int three = 0;
	int total = 10000;
	for (i = 0; i < total; i++){
		num = rand();
		if (num > 30000) three++;
		else if (num > 20000) two++;
		else if (num > 10000) one++;
		else zero++;
		printf("%d\n",num);
	}
	printf("generated %d numbers in total\n",total);
	printf("0-10000:     %d\n",zero);
	printf("10000-20000: %d\n",one);
	printf("20000-30000: %d\n",two);
	printf("30000-32767: %d\n",three);

	printf("press q to quit this test\n");
	while (display_terminal->keyboard_buf[display_terminal->read_count-1] != 'q');
	clear_keyboard_buffer();
	display_terminal->read_count = 0;
	printf("\n");

	return PASS;
}

int beep_test(){
	TEST_HEADER;

	int i;
	uint32_t freq[8] = {256,288,320,341,384,427,480,512};
	for (i = 0; i < 8; i++){
		beep(freq[i]);
	}

	printf("press q to quit this test\n");
	while (display_terminal->keyboard_buf[display_terminal->read_count-1] != 'q');
	clear_keyboard_buffer();
	display_terminal->read_count = 0;
	printf("\n");

	return PASS;
}

int play_wav_test(){
	TEST_HEADER;

	enable_irq(5);
	fd_t tmp_fd_array[MAX_FILES];
    init_fd(tmp_fd_array);
	play_music((uint8_t*)"music");
	timer_wait(4);
	stop();
	disable_irq(5);

	printf("press q to quit this test\n");
	while (display_terminal->keyboard_buf[display_terminal->read_count-1] != 'q');
	clear_keyboard_buffer();
	display_terminal->read_count = 0;
	printf("\n");

	return PASS;
}

/* pause
 * a helper function
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
int32_t pause(){
	printf("Press c to continue, or q to quit.\n");
	uint8_t buffer[128];
	while (1){
		int32_t num_read = terminal_read(0, buffer, 128);
		if (num_read != 2) continue;
		if (buffer[0] == 'c') return 1;
		if (buffer[0] == 'q') return -1;
	}
}



/* str2num
 * a helper function
 * Inputs: a pointer to a buffer
 * Outputs: None
 * Side Effects: None
 */
int32_t str2num(uint8_t* buf){
	int32_t num_read = terminal_read(0, buf, 128);
	int32_t i, j;
	int32_t counter = 0;
	for (i = 0; i < num_read - 1; ++i){
		if (buf[i] >= '0' && buf[i] <= '9'){
			int32_t temp = (int32_t)(buf[i] - '0');
			for (j = 0; j < num_read - 2 - i; ++j){
				temp *= 10;
			}
			counter += temp;
		}
		else{
			return -1;
		}
	}
	return counter;
}



/* test_DA
 * 
 * Test dynamic allocation.
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void test_DA(){
	current_color = 0;
	terminal_video();

	uint8_t buffer[128];
	int32_t num_region = 0;
	int32_t i;	// loop index
	int32_t num_byte;
	void* temp_ptr;

	while (1){
		printf("\n********************\n");
		printf("Enter the number of allocated regions (1 - 20).\n");
		num_region = str2num(buffer);
		if (num_region < 1 || num_region > 20){
			printf("Wrong input, let's use 3 instead.\n");
			num_region = 3;
		}

		for (i = 0; i < num_region; ++i){
			printf("\nEnter the number of bytes for memory chunk %d\n", i);
			num_byte = str2num(buffer);
			if (num_byte == -1){
				printf("Wrong input, let's use 42 instead.\n");
				num_byte = 42;
			}
			temp_ptr = malloc(num_byte);
			if (temp_ptr == NULL){
				printf("Allocation failed.\n");
			}
			else{
				printf("Allocated successfully (%d bytes).\n", num_byte);
			}
		}
		printf("\n********************\n");
		printf("Heap-related info:\n");
		print_heap_info();

		printf("\n********************\n");
		printf("Want to free any memory chunk?\n");
		while (1){
			printf("Enter a pointer(address), or enter -1 to quit.\n");
			i = str2num(buffer);
			if (i == -1) break;
			free((void*)i);
		}
		printf("\n********************\n");
		printf("Let's see what is left in the memory.\n");
		print_heap_info();

		int32_t res = pause();
		if (res == -1)	break;
	}

	printf("Do you want to check if any boundary bug occurs?\n");
	printf("Enter 1 if you want.\n");
	i = str2num(buffer);
	if (i == 1){
		while (1){
			printf("Enter the address of the memory chunk which you want to test, or -1 to quit.\n");
			i = str2num(buffer);
			if (i == -1) break;
			int32_t res = validate((void*)i);
			if (res == 0){
				printf("All good.\n");
			}
			else{
				printf("Boundary bug detected.\n");
			}
		}
	}

	current_color = -1;
}

void launch_tests(){
	current_color = 0;
    terminal_video();

	// TEST_OUTPUT("random_test1", random_test1());
	// TEST_OUTPUT("random_test2", random_test2());
	// TEST_OUTPUT("beep_test", beep_test());
	TEST_OUTPUT("play_wav_test", play_wav_test());

	// test_DA();

	current_color = -1;
}
#endif
