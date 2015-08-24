#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/fail.h>

#include <maxminddb.h>

CAMLprim value mmdb_ml_version(value __unused v_unit)
{
	return caml_copy_string(MMDB_lib_version());
}

CAMLprim value mmdb_ml_open(value s)
{
	char *as_string = String_val(s);
	// this needs to be a special one.
	value raw = caml_alloc(sizeof(MMDB_s), Abstract_tag);
	MMDB_s *as_mmdb = (MMDB_s*)Data_custom_val(raw);

	int status = MMDB_open(as_string, MMDB_MODE_MMAP, as_mmdb);

	if (MMDB_SUCCESS != status) {
		fprintf(stderr, "\n  Can't open %s - %s\n",
			as_string, MMDB_strerror(status));

		if (MMDB_IO_ERROR == status) {
			fprintf(stderr, "    IO error: %s\n", strerror(errno));
		}
		exit(1);
	}
	//return (value)as_mmdb;
	return raw;
}

CAMLprim value mmdb_ml_close(value mmdb)
{
	MMDB_s *as_mmdb = (MMDB_s*)Data_custom_val(mmdb);
	MMDB_close(as_mmdb);
	return Val_unit;
}

static char* pull_all_data(FILE *f)
{
	int size = 0;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	/* printf("Size: %d\n", size); */
	rewind(f);
	char *buffer = malloc(size);
	fread(buffer, size, 1, f);
	fflush(f);
	/* printf("Check:%s\n", buffer); */
	return buffer;
}

CAMLprim value mmdb_ml_lookup_string(value ip, value mmdb)
{
	char *as_string = String_val(ip);
	MMDB_s *as_mmdb = (MMDB_s*)Data_custom_val(mmdb);
	int gai_error, mmdb_error;
	value raw = caml_alloc(sizeof(MMDB_lookup_result_s), Abstract_tag);
	MMDB_lookup_result_s *result =
		(MMDB_lookup_result_s*)Data_custom_val(raw);
	*result =
		MMDB_lookup_string(as_mmdb, as_string, &gai_error, &mmdb_error);

	MMDB_entry_data_list_s *entry_data_list = NULL;

	int status = MMDB_get_entry_data_list(&result->entry, &entry_data_list);

	if (status != MMDB_SUCCESS) {
		printf("Something messed up, need to raise exception\n");
	}
	char temp_name[] = "/tmp/ocaml-maxminddb-XXXXXX";
	mkstemp(temp_name);
	FILE *f = fopen(temp_name, "r+wb");
	MMDB_dump_entry_data_list(f, entry_data_list, 2);
	char *pulled_from_db = pull_all_data(f);
	fclose(f);
	remove(temp_name);
	return caml_copy_string(pulled_from_db);
}
