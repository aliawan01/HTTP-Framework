#include "util.h"

bool HTTP_FileExists(char* file_name) {
    FILE* file = fopen(file_name, "rb");
    bool result;
    if (file == NULL) {
        result = false;
    }
    else {
        result = true;
        fclose(file);
    }

    return result;
}

bool HTTP_DeleteFile(char* file_name) {
    bool success = false;

    if (!remove(file_name)) {
        printf("[INFO] HTTP_DeleteFile() Successfully deleted file with file name: `%s`\n", file_name);
        success = true;
    }
    else {
        printf("[ERROR] HTTP_DeleteFile() Unable to delete file with file name: `%s`.\n", file_name);
    }

    return success;
}

int64_t HTTP_FindFileSize(char *file_name) {
	FILE* file = fopen(file_name, "rb");
	if (file == NULL) {
		printf("FindFileSize(): Error couldn't open the file %s\n", file_name);
		return -1;
	}

	fseek(file, 0, SEEK_END);
	int64_t file_size = ftell(file);
	fclose(file);
	return file_size;
}

char* HTTP_GetFileContents(Arena* arena, char* file_name) {
	FILE* file = fopen(file_name, "rb");
	if (file == NULL) {
		printf("GetFileContents(): Error couldn't open the file %s\n", file_name);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	int64_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* file_contents = PushString(arena, file_size+1);

	fread(file_contents, sizeof(char), file_size, file);
	fclose(file);
	return file_contents;
}

bool HTTP_OverwriteFileContents(char* file_name, char* new_file_contents) {
	FILE* file = fopen(file_name, "wb+");
	if (file == NULL) {
		printf("OverwriteFileContents(): Error couldn't open the file %s\n", file_name);
		return false;
	}

    fwrite(new_file_contents, sizeof(char), strlen(new_file_contents), file);
	fclose(file);
    return true;
}
