#include "http_util.h"
#include "http_file_handling.h"

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
        HTTP_Log(HTTP_INFO, "[INFO] HTTP_DeleteFile() Successfully deleted file with file name: `%s`\n", file_name);
        success = true;
    }
    else {
        HTTP_Log(HTTP_ERROR, "[ERROR] HTTP_DeleteFile() Unable to delete file with file name: `%s`.\n", file_name);
    }

    return success;
}

int64_t HTTP_FindFileSize(char *file_name) {
	FILE* file = fopen(file_name, "rb");
	if (file == NULL) {
		HTTP_Log(HTTP_ERROR, "FindFileSize(): Error couldn't open the file %s\n", file_name);
		return -1;
	}

	fseek(file, 0, SEEK_END);
	int64_t file_size = ftell(file)+1;
	fclose(file);
	return file_size;
}

String HTTP_GetFileContents(Arena* arena, char* file_name) {
    String file_contents = {0};

	FILE* file = fopen(file_name, "rb");
	if (file == NULL) {
		HTTP_Log(HTTP_ERROR, "GetFileContents(): Error couldn't open the file %s\n", file_name);
		return file_contents;
	}

	fseek(file, 0, SEEK_END);
	file_contents.count = ftell(file);
	fseek(file, 0, SEEK_SET);

	file_contents.string = PushString(arena, file_contents.count+1);

	fread(file_contents.string, sizeof(char), file_contents.count+1, file);
	fclose(file);

	return file_contents;
}

bool HTTP_OverwriteFileContents(char* file_name, char* new_file_contents) {
	FILE* file = fopen(file_name, "wb+");
	if (file == NULL) {
		HTTP_Log(HTTP_ERROR, "OverwriteFileContents(): Error couldn't open the file %s\n", file_name);
		return false;
	}

    fwrite(new_file_contents, sizeof(char), strlen(new_file_contents), file);
	fclose(file);
    return true;
}
