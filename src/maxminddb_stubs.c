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

#define Val_none Val_int(0)

static value
Val_some(value v)
{
  CAMLparam1(v);
  CAMLlocal1(some);
  some = caml_alloc(1, 0);
  Store_field(some, 0, v);
  CAMLreturn(some);
}

#include <maxminddb.h>

static char* pull_all_data(FILE *f)
{
  int size = 0;
  fseek(f, 0, SEEK_END);
  fflush(f);
  size = ftell(f);
  rewind(f);
  fflush(f);
  char *buffer = malloc(size);
  fread(buffer, size, 1, f);
  fflush(f);
  return buffer;
}

void check_error(int gai_error, int mmdb_error)
{
  if (gai_error) {
    caml_failwith((const char *)MMDB_strerror(gai_error));
  }
  if (mmdb_error) {
    caml_failwith((const char *)MMDB_strerror(mmdb_error));
  }
}

void check_status(int status)
{
  if (MMDB_SUCCESS != status) {
    caml_invalid_argument((const char *)MMDB_strerror(status));
  }
}

void check_data(MMDB_entry_data_s entry_data)
{
  if (!entry_data.has_data)
    caml_failwith("No data available");
}

char *data_from_dump(MMDB_entry_data_list_s *entry_data_list)
{
  char temp_name[] = "/tmp/ocaml-maxminddb-XXXXXX";
  mkstemp(temp_name);
  FILE *f = fopen(temp_name, "r+wb");
  MMDB_dump_entry_data_list(f, entry_data_list, 2);
  fflush(f);
  char *pulled_from_db = pull_all_data(f);
  fclose(f);
  remove(temp_name);
  return pulled_from_db;
}

CAMLprim value mmdb_ml_version(value v_unit)
{
  CAMLparam1(v_unit);
  CAMLlocal1(ml_v_string);

  const char *v = MMDB_lib_version();
  ml_v_string = caml_copy_string(v);
  CAMLreturn(ml_v_string);
}

CAMLprim value mmdb_ml_open(value s)
{
  CAMLparam1(s);
  CAMLlocal1(raw);

  char *as_string = String_val(s);
  raw = caml_alloc(sizeof(MMDB_s), Abstract_tag);
  MMDB_s *as_mmdb = (MMDB_s*)Data_custom_val(raw);
  int status = MMDB_open(as_string, MMDB_MODE_MMAP, as_mmdb);
  check_status(status);
  CAMLreturn(raw);
}

CAMLprim value mmdb_ml_close(value mmdb)
{
  CAMLparam1(mmdb);

  MMDB_s *as_mmdb = (MMDB_s*)Data_custom_val(mmdb);
  MMDB_close(as_mmdb);
  CAMLreturn(Val_unit);
}

CAMLprim value mmdb_ml_dump_global(value mmdb)
{
  CAMLparam1(mmdb);
  CAMLlocal2(raw, pulled_string);

  MMDB_s *as_mmdb = (MMDB_s*)Data_custom_val(mmdb);
  MMDB_entry_data_list_s *entry_data_list = NULL;

  int status = MMDB_get_metadata_as_entry_data_list(as_mmdb,
						    &entry_data_list);
  check_status(status);
  char *pulled_from_db = data_from_dump(entry_data_list);
  pulled_string = caml_copy_string(pulled_from_db);
  free(pulled_from_db);
  CAMLreturn(pulled_string);
}

CAMLprim value mmdb_ml_dump_per_ip(value ip, value mmdb)
{
  CAMLparam2(ip, mmdb);
  CAMLlocal2(raw, pulled_string);

  char *as_string = String_val(ip);
  MMDB_s *as_mmdb = (MMDB_s*)Data_custom_val(mmdb);
  int gai_error, mmdb_error;
  raw = caml_alloc(sizeof(MMDB_lookup_result_s), Abstract_tag);
  MMDB_lookup_result_s *result = (MMDB_lookup_result_s*)Data_custom_val(raw);
  *result = MMDB_lookup_string(as_mmdb, as_string, &gai_error, &mmdb_error);
  MMDB_entry_data_list_s *entry_data_list = NULL;
  int status = MMDB_get_entry_data_list(&result->entry, &entry_data_list);
  check_status(status);
  char *pulled_from_db = data_from_dump(entry_data_list);
  pulled_string = caml_copy_string(pulled_from_db);
  free(pulled_from_db);
  CAMLreturn(pulled_string);
}

CAMLprim value mmdb_ml_lookup_path(value ip, value query_list, value mmdb)
{
  CAMLparam3(ip, query_list, mmdb);
  CAMLlocal3(iter_count, raw, caml_clean_result);
  int total_len = 0, copy_count = 0, gai_error = 0, mmdb_error = 0;
  iter_count = query_list;
  char *ip_as_str = String_val(ip);
  MMDB_s *as_mmdb = (MMDB_s*)Data_custom_val(mmdb);

  raw = caml_alloc(sizeof(MMDB_lookup_result_s), Abstract_tag);
  MMDB_lookup_result_s *result =
    (MMDB_lookup_result_s*)Data_custom_val(raw);
  *result = MMDB_lookup_string(as_mmdb, ip_as_str, &gai_error, &mmdb_error);
  check_error(gai_error, mmdb_error);

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

  int status = MMDB_aget_value(&result->entry,
			       &entry_data,
			       (const char *const *const)query);
  check_status(status);
  check_data(entry_data);
  char *clean_result;

  switch (entry_data.type) {
  case MMDB_DATA_TYPE_BYTES: {
    clean_result = malloc(entry_data.data_size + 1);
    memcpy(clean_result, entry_data.bytes, entry_data.data_size);
    break;
  }
  case MMDB_DATA_TYPE_UTF8_STRING: {
    clean_result = malloc(entry_data.data_size + 1);
    memcpy(clean_result, entry_data.utf8_string, entry_data.data_size);
    break;
  }
  case MMDB_DATA_TYPE_FLOAT: {
    clean_result = malloc(48);
    sprintf(clean_result, "%f", entry_data.float_value);
    break;
  }
  case MMDB_DATA_TYPE_BOOLEAN: {
    clean_result = malloc((entry_data.boolean ? 4 : 5) + 1);
    sprintf(clean_result, "%s", entry_data.boolean ? "true" : "false");
    break;
  }
  case MMDB_DATA_TYPE_DOUBLE: {
    clean_result = malloc(48);
    sprintf(clean_result, "%f", entry_data.double_value);
    break;
  }
  case MMDB_DATA_TYPE_UINT16: {
    clean_result = malloc(5 + 1);
    sprintf(clean_result, "%d", entry_data.uint16);
    break;
  }
  case MMDB_DATA_TYPE_UINT32: {
    clean_result = malloc(10 + 1);
    sprintf(clean_result, "%d", entry_data.uint32);
    break;
  }
  case MMDB_DATA_TYPE_UINT64: {
    clean_result = malloc(20 + 1);
    sprintf(clean_result, "%d", entry_data.uint64);
    break;
  }
    // look at /usr/bin/sed -n 1380,1430p src/maxminddb.c
  case MMDB_DATA_TYPE_ARRAY:
  case MMDB_DATA_TYPE_MAP:
    caml_failwith("Can't return a Map or Array yet");
  }

  caml_clean_result = caml_copy_string(clean_result);
  free(query);
  free(clean_result);
  CAMLreturn(caml_clean_result);
}
