#pragma once

static int HTTP_FindFileSize(char* file_name) {
	FILE* file = fopen(file_name, "rb");
	if (file == NULL) {
		printf("FindFileSize(): Error couldn't open the file %s\n", file_name);
		return -1;
	}

	fseek(file, 0, SEEK_END);
	int file_size = ftell(file);
	fclose(file);
	return file_size;
}

static char* HTTP_GetFileContents(char* file_name) {
	FILE* file = fopen(file_name, "rb");
	if (file == NULL) {
		printf("GetFileContents(): Error couldn't open the file %s\n", file_name);
		// TODO: Fix this.
		return "error";
	}

	fseek(file, 0, SEEK_END);
	int file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* file_contents = malloc(file_size+1);
	memset(file_contents, 0, file_size+1);

	fread(file_contents, sizeof(char), file_size, file);
	fclose(file);
	return file_contents;
}
