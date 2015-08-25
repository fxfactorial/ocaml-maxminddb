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

CAMLprim value mmdb_ml_version(value v_unit)
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

CAMLprim value mmdb_ml_dump(value ip, value mmdb)
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
		caml_failwith("Could not do a dump of the Database");
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

CAMLprim value mmdb_ml_lookup_path(value ip, value query_list, value mmdb)
{
	int total_len = 0, copy_count = 0, gai_error, mmdb_error;
	value iter_count = query_list;
	char *ip_as_str = String_val(ip);
	MMDB_s *as_mmdb = (MMDB_s*)Data_custom_val(mmdb);

	value raw = caml_alloc(sizeof(MMDB_lookup_result_s), Abstract_tag);
	MMDB_lookup_result_s *result =
		(MMDB_lookup_result_s*)Data_custom_val(raw);
	*result =
		MMDB_lookup_string(as_mmdb, ip_as_str, &gai_error, &mmdb_error);

	while (iter_count != Val_emptylist) {
		total_len++;
		iter_count = Field(iter_count, 1);
	}

	char **query = malloc(sizeof(char *) * (total_len + 1));

	while (query_list != Val_emptylist) {
		query[copy_count] = String_val(Field(query_list, 0));
		copy_count++;
		query_list = Field(query_list, 1);
	}
	query[total_len] = NULL;
	MMDB_entry_data_s entry_data;

	int status = MMDB_aget_value(&result->entry, &entry_data, (const char *const *const)query);

	if (MMDB_SUCCESS != status) {
		caml_invalid_argument("Bad lookup path provided");
	}

	char clean_result[entry_data.data_size];
	memcpy(clean_result, entry_data.bytes, entry_data.data_size);
	return caml_copy_string(clean_result);
}
