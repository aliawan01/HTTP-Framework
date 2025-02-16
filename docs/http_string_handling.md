# http\_string\_handling.h



----------

## `HTTP_StringDup`

### Description
Duplicates a string and stores the copy in the specified Arena.

### Parameters
- `arena` *(Arena\*)* -  arena The Arena to store the duplicated string.

- `source` *(char\*)* -  source The source string to duplicate.

### Returns
- `char*` - Returns a pointer to the newly duplicated string being stored in the arena.