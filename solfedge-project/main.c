#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#define SUCCESS 0
#define ERRNO_ALLOC 505
#define ALLOC(sz,p) if (SUCCESS != cgc_allocate(sz, 0, (void **)p)) {cgc__terminate(ERRNO_ALLOC);}
// 4096 syllables bytes, assuming 2 bytes per syllable is 2048 syllables
// this assumption is wrong because one syllable is 3 bytes.
#define MAX_SYLLABLES_BYTES 4096
// 2048 notes bytes, assyming 1 byte per note is 2048 notes
// this assumption is correct. However, when the notes
// get converted to syllables, if any note relates to the one
// 3 byte syllable, then the total syllable bytes will exceed 4096 bytes.
#define MAX_NOTES_BYTES 2048
#define BUFFER_LEN 4096 
#define STDIN 0

typedef unsigned long cgc_size_t;
typedef long cgc_ssize_t;
typedef signed char int8_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

// notes mapping
enum {
	C = 1,
	D = 2,
	E = 3,
	F = 4,
	G = 5,
	A = 6,
	B = 7,
};

// syllables mapping
enum {
	Ut = 1,
	Re = 2,
	Mi = 3,
	Fa = 4,
	Sol = 5,
	La = 6,
	Si = 7,
};

// error codes 
enum {
	//ERR_RECV_FAILED = -900,
	ERR_INVALID_CMD = -901,
	ERR_INVALID_NOTE = -902,
	ERR_INVALID_SYLLABLE = -903,
	ERR_TOO_MANY_NOTES = -904,
	ERR_TOO_MANY_SYLLABLES = -905,
	ERR_NO_NOTES = -906,
	ERR_NO_SYLLABLES = -907,
	ERR_OPENING_FILE = -908
};

// translation modes
enum {
    CMD_TO_SYLLABLES = 804619,
    CMD_TO_NOTES = 1128809,
};

void cgc__terminate(unsigned int status) {
	exit(status);
}

int cgc_allocate(cgc_size_t length, int is_executable, void** addr) {
	DWORD prot = is_executable ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE;

	LPVOID ret_addr = VirtualAlloc(NULL, length, MEM_COMMIT | MEM_RESERVE, prot);
	if (ret_addr == NULL)
		return GetLastError();

	if (addr != NULL)
		*addr = ret_addr;

	return 0;
}

void* cgc_memset(void* dst, int c, unsigned int n) {
	char* d = (char*)dst;
	while (n--) { *d++ = (char)c; }
	return dst;
}

/*
 * Convert the note to the associated syllable and save the
 * syllable into syllable_buf.
 *
 * Returns:
 *  Success: 2 or 3 (number of letters written to syllable_buf)
 *  Failure: ERR_INVALID_NOTE
 */
int cgc_get_syllable_for_note_id(int note_id, char* syllable_buf) {

	switch (note_id) {
	case C:
		syllable_buf[0] = 'U';
		syllable_buf[1] = 't';
		return 2;
	case D:
		syllable_buf[0] = 'R';
		syllable_buf[1] = 'e';
		return 2;
	case E:
		syllable_buf[0] = 'M';
		syllable_buf[1] = 'i';
		return 2;
	case F:
		syllable_buf[0] = 'F';
		syllable_buf[1] = 'a';
		return 2;
	case G:
		syllable_buf[0] = 'S';
		syllable_buf[1] = 'o';
		syllable_buf[2] = 'l';
		return 3;
	case A:
		syllable_buf[0] = 'L';
		syllable_buf[1] = 'a';
		return 2;
	case B:
		syllable_buf[0] = 'S';
		syllable_buf[1] = 'i';
		return 2;
	default:
		return ERR_INVALID_NOTE;
	}
}

/*
 * Convert the syllable to the associated note and save the
 * note into note_buf.
 *
 * If syllable is invalid, note_buf is undefined.
 *
 * Returns:
 *  Success: SUCCESS
 *  Failure: ERR_INVALID_SYLLABLE
 */
int cgc_get_note_for_syllable_id(int syllable_id, char* note_buf) {

	switch (syllable_id) {
	case Ut:
		note_buf[0] = 'C';
		return SUCCESS;
	case Re:
		note_buf[0] = 'D';
		return SUCCESS;
	case Mi:
		note_buf[0] = 'E';
		return SUCCESS;
	case Fa:
		note_buf[0] = 'F';
		return SUCCESS;
	case Sol:
		note_buf[0] = 'G';
		return SUCCESS;
	case La:
		note_buf[0] = 'A';
		return SUCCESS;
	case Si:
		note_buf[0] = 'B';
		return SUCCESS;
	default:
		return ERR_INVALID_SYLLABLE;
	}
}

/*
 * Read the string and return the id of the first note.
 *
 * str is not expected to be null terminated.
 *
 * Returns:
 *  Success: 1 thru 7 from notes enum
 *  Failure: ERR_INVALID_NOTE
 */
int cgc_get_next_note_id(const char* str) {

	switch (str[0]) {
	case 'C':
		return C;
	case 'D':
		return D;
	case 'E':
		return E;
	case 'F':
		return F;
	case 'G':
		return G;
	case 'A':
		return A;
	case 'B':
		return B;
	default:
		return ERR_INVALID_NOTE;
	}
}

/*
 * Read the string and return the id of the
 * syllable at the beginning of the string.
 *
 * bytes_read is a 1 byte char buffer.
 * str is not expected to be null terminated.
 *
 * If an invaild syllable is encountered, bytes_read is undefined.
 *  And the syllables in the remainder of the string are undefined.
 *
 * Returns:
 *  Success: 1 thru 7 in syllables enum
 *  Failure: ERR_INVALID_SYLLABLE
 */
int cgc_get_next_syllable_id(const char* str, char* bytes_read) {

	char s0 = str[0];
	char s1 = str[1];
	char s2 = str[2];
	if ('U' == s0 && 't' == s1) {
		bytes_read[0] = 2;
		return Ut;
	}
	else 	if ('R' == s0 && 'e' == s1) {
		bytes_read[0] = 2;
		return Re;
	}
	else 	if ('M' == s0 && 'i' == s1) {
		bytes_read[0] = 2;
		return Mi;
	}
	else 	if ('F' == s0 && 'a' == s1) {
		bytes_read[0] = 2;
		return Fa;
	}
	else 	if ('S' == s0 && 'o' == s1 && 'l' == s2) {
		bytes_read[0] = 3;
		return Sol;
	}
	else 	if ('L' == s0 && 'a' == s1) {
		bytes_read[0] = 2;
		return La;
	}
	else 	if ('S' == s0 && 'i' == s1) {
		bytes_read[0] = 2;
		return Si;
	}
	else {
		return ERR_INVALID_SYLLABLE;
	}
}

/*
 * Write the note matching syllable_id into notes_buf.
 *
 * Returns:
 *  Success: 1 (number of bytes written)
 *  Failure: ERR_INVALID_SYLLABLE
 */
int cgc_write_note_to_buf(int syllable_id, char* notes_buf) {

	int ret = 1;
	char note = 0;

	ret = cgc_get_note_for_syllable_id(syllable_id, &note);
	if (SUCCESS == ret) {
		notes_buf[0] = note;
		ret = 1;
	}
	return ret;
}

/*
 * Write the syllable matching note_id into syllable_buf.
 *
 * Returns:
 *  Success: 2 or 3 (number of bytes written)
 *  Failure: ERR_INVALID_NOTE
 */
int cgc_write_syllable_to_buf(int note_id, char* syllable_buf) {

	int ret = 2;
	char syllable[3] = { 0 };

	ret = cgc_get_syllable_for_note_id(note_id, syllable);
	if (0 < ret) {
		syllable_buf[0] = syllable[0];
		syllable_buf[1] = syllable[1];
	}
	if (3 == ret) {
		syllable_buf[2] = syllable[2];
	}
	return ret;
}

/*
 * Loop through syllables in syllables_buf, convert them to notes and
 *  cgc_write them to notes_buf.
 *
 * Processing will stop when either an invalid syllable is found,
 * or bytes_count syllables have been processed.
 *
 * Returns:
 *  Success: total bytes written to notes_buf (> 0)
 *  Failure: ERR_INVALID_SYLLABLE
 */
int cgc_process_syllables(uint32_t bytes_count, char* syllables_buf, char* notes_buf) {

	int ret = 1;
	char* s_buf_ptr = syllables_buf;
	char* n_buf_ptr = notes_buf;
	int syllable_id = 0;
	char bytes_read[1] = { 0 };
	int total_bytes_written = 0;

	while ((0 < ret) && (0 < bytes_count)) {
		syllable_id = cgc_get_next_syllable_id(s_buf_ptr, bytes_read);
		if (0 < syllable_id) {
			s_buf_ptr += bytes_read[0];
			bytes_count -= bytes_read[0];

			ret = cgc_write_note_to_buf(syllable_id, n_buf_ptr);
			if (1 == ret) {
				n_buf_ptr += ret;
				total_bytes_written += ret;
			}
		}
		else {
			ret = syllable_id;
		}

	}

	// ret == 0 not possible.
	if (0 < ret) {
		ret = total_bytes_written;
	}

	return ret;
}

/*
 * Loop through notes in notes_buf, convert them to syllables and
 *  cgc_write them to syllables_buf.
 *
 * Processing will stop when either an invalid note is found,
 * or bytes_count notes have been processed.
 *
 * Returns:
 *  Success: total bytes written to syllables_buf (> 0)
 *  Failure: ERR_INVALID_NOTE, ERR_TOO_MANY_NOTES
 */
int cgc_process_notes(uint32_t bytes_count, char* syllables_buf, char* notes_buf) {

	int ret = 1;
	char* s_buf_ptr = syllables_buf;
	char* n_buf_ptr = notes_buf;
	int note_id = 0;
	int total_bytes_written = 0;

#if PATCHED
	while ((0 < ret) && ((MAX_SYLLABLES_BYTES - 2) > total_bytes_written) && (0 < bytes_count)) {
#else
	while ((0 < ret) && (MAX_SYLLABLES_BYTES > total_bytes_written) && (0 < bytes_count)) {
#endif
		note_id = cgc_get_next_note_id(n_buf_ptr);
		if (0 < note_id) {
			n_buf_ptr++;
			bytes_count--;

			ret = cgc_write_syllable_to_buf(note_id, s_buf_ptr);
			if ((2 == ret) || (3 == ret)) {
				s_buf_ptr += ret;
				total_bytes_written += ret;
			}
		}
		else {
			ret = note_id;
		}
	}

	// ret == 0 not possible.
	if (0 < ret) {
		ret = total_bytes_written;
	}

	return ret;
	}


int main(int argc, char* argv[]) {	
	int ret = 0;
	
	if (argc < 3) {
		printf("Usage: %s <translation mode> <input file>\n", argv[0]);
	}
	else {
		char* syllables_buf_p;
		ALLOC(BUFFER_LEN, &syllables_buf_p);

		char* notes_buf_p;
		ALLOC(BUFFER_LEN, &notes_buf_p);

		long i = strtol(argv[1], NULL, 10);

		if (i < 0 || i > INT_MAX) {
			ret = ERR_INVALID_CMD;
			return ret;
		}

		uint32_t command[1] = { (uint32_t)i };
		// open file for reading
		FILE* fp;
		errno_t err;
		err = fopen_s(&fp, argv[2], "r+");
		if (!fp) {
			ret = ERR_OPENING_FILE;
			return ret;
		}

		// determine no of bytes 
		fseek(fp, 0, SEEK_END);
		long int bytes_count = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		switch (command[0]) {
		case CMD_TO_SYLLABLES:
			if (bytes_count <= 0)
			{
				fclose(fp);
				return ERR_NO_NOTES;
			}
			if (MAX_NOTES_BYTES < bytes_count)
			{
				fclose(fp);
				return ERR_TOO_MANY_NOTES;
			}
			else {
				fread(notes_buf_p, sizeof(char), bytes_count, fp);
				int total_bytes_written = cgc_process_notes(bytes_count, syllables_buf_p, notes_buf_p);

				if (0 < total_bytes_written) {
					printf("% .*s", MAX_SYLLABLES_BYTES, syllables_buf_p);
					ret = SUCCESS;
				}
				else if (0 == total_bytes_written) {
					ret = ERR_NO_NOTES;
				}
				else {
					ret = total_bytes_written;
				}
			}
			break;

		case CMD_TO_NOTES:
			if (bytes_count <= 0)
			{
				fclose(fp);
				return ERR_NO_SYLLABLES;
			}
			if (MAX_NOTES_BYTES < bytes_count)
			{
				fclose(fp);
				return ERR_TOO_MANY_SYLLABLES;
			}
			else {
				fread(syllables_buf_p, sizeof(char), bytes_count, fp);
				int total_bytes_written = cgc_process_syllables(bytes_count, syllables_buf_p, notes_buf_p);

				if (0 < total_bytes_written) {
					printf("% .*s", MAX_NOTES_BYTES, syllables_buf_p);
					ret = SUCCESS;
				}
				else if (0 == total_bytes_written) {
					ret = ERR_NO_SYLLABLES;
				}
				else {
					ret = total_bytes_written;
				}
			}
			break;

		default:
			ret = ERR_INVALID_CMD;
		}
		fclose(fp);

		if (ret < 0) {
			return ret;
		}
		else {
			cgc_memset((void*)syllables_buf_p, 0, BUFFER_LEN);
			cgc_memset((void*)notes_buf_p, 0, BUFFER_LEN);
		}
	}
	return ret;
}