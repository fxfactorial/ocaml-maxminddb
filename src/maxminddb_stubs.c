// C Standard stuff
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
// OCaml declarations
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/fail.h>
// libmaxminddb
#include <maxminddb.h>

// Because who knows what the hash will be across platforms, compiler
// versions, whatever.
static struct {
  long poly_string;
  long poly_int;
  long poly_float;
  long poly_bool;
} polymorphic_variants = {0, 0, 0, 0};

static char* pull_all_data(FILE *f)
{
  int size = 0;
  fseek(f, 0, SEEK_END);
  fflush(f);
  size = ftell(f);
  rewind(f);
  fflush(f);
  char *buffer = caml_stat_alloc(size);
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
  if (!f) {
    caml_failwith("Couldn't open tempfile needed for dumping database");
  }
  MMDB_dump_entry_data_list(f, entry_data_list, 2);
  fflush(f);
  char *pulled_from_db = pull_all_data(f);
  fclose(f);
  remove(temp_name);
  return pulled_from_db;
}

CAMLprim value mmdb_ml_version(void)
{
  return caml_copy_string(MMDB_lib_version());
}

CAMLprim value mmdb_ml_open(value s)
{
  CAMLparam1(s);
  CAMLlocal1(record);

  if (polymorphic_variants.poly_bool == 0  ||
      polymorphic_variants.poly_float == 0 ||
      polymorphic_variants.poly_int == 0   ||
      polymorphic_variants.poly_string == 0) {
    polymorphic_variants.poly_bool = caml_hash_variant("Bool");
    polymorphic_variants.poly_float = caml_hash_variant("Float");
    polymorphic_variants.poly_int = caml_hash_variant("Int");
    polymorphic_variants.poly_string = caml_hash_variant("String");
  }

  unsigned int len = caml_string_length(s);
  char *copied = caml_strdup(String_val(s));
  if (strlen(copied) != (size_t)len) {
    caml_failwith("Could not open MMDB database");
  }

  record = caml_alloc(2, 0);
  MMDB_s *this_db = caml_stat_alloc(sizeof(*this_db));
  int status = MMDB_open(copied, MMDB_MODE_MMAP, this_db);
  check_status(status);
  Store_field(record, 0, 1);
  Store_field(record, 1, (value)this_db);
  free(copied);
  CAMLreturn(record);
}

CAMLprim value mmdb_ml_close(value record)
{
  CAMLparam1(record);
  CAMLlocal1(init_value);

  init_value = Field(record, 0);

  if (Val_bool(init_value) == Val_false) {
    caml_failwith("Can't close an already closed DB handle");
  }

  MMDB_s *this_db = (MMDB_s*)Field(record, 1);
  MMDB_close(this_db);
  free(this_db);
  Store_field(record, 0, 0);
  CAMLreturn(Val_unit);
}

CAMLprim value mmdb_ml_dump_global(value mmdb)
{
  CAMLparam1(mmdb);
  CAMLlocal2(this_db, pulled_string);

  this_db = Field(mmdb, 1);
  MMDB_s *as_mmdb = (MMDB_s*)this_db;
  MMDB_entry_data_list_s *entry_data_list = NULL;
  int status = MMDB_get_metadata_as_entry_data_list(as_mmdb, &entry_data_list);
  check_status(status);
  char *pulled_from_db = data_from_dump(entry_data_list);
  pulled_string = caml_copy_string(pulled_from_db);
  free(pulled_from_db);
  CAMLreturn(pulled_string);
}

CAMLprim value mmdb_ml_dump_per_ip(value ip, value mmdb)
{
  CAMLparam2(ip, mmdb);
  CAMLlocal2(pulled_string, db);

  unsigned int len = caml_string_length(ip);
  char *as_string = caml_strdup(String_val(ip));

  if (strlen(as_string) != (size_t)len) {
    caml_failwith("Could not copy IP address properly");
  }

  db = Field(mmdb, 1);
  MMDB_s *as_mmdb = (MMDB_s*)db;
  int gai_error = 0, mmdb_error = 0;

  MMDB_lookup_result_s *result = caml_stat_alloc(sizeof(*result));
  *result = MMDB_lookup_string(as_mmdb, as_string, &gai_error, &mmdb_error);
  MMDB_entry_data_list_s *entry_data_list = NULL;
  int status = MMDB_get_entry_data_list(&result->entry, &entry_data_list);
  check_status(status);
  char *pulled_from_db = data_from_dump(entry_data_list);
  pulled_string = caml_copy_string(pulled_from_db);
  free(result);
  free(as_string);
  free(pulled_from_db);
  CAMLreturn(pulled_string);
}

CAMLprim value mmdb_ml_lookup_path(value ip, value query_list, value mmdb)
{
  CAMLparam3(ip, query_list, mmdb);
  CAMLlocal4(iter_count, caml_clean_result, query_r, db);

  int total_len = 0, copy_count = 0, gai_error = 0, mmdb_error = 0;
  char *clean_result;
  long int int_result;

  iter_count = query_list;

  unsigned int len = caml_string_length(ip);
  char *as_string = caml_strdup(String_val(ip));
  if (strlen(as_string) != (size_t)len) {
    caml_failwith("Could not copy IP address properly");
  }
  db = Field(mmdb, 1);
  MMDB_s *as_mmdb = (MMDB_s*)db;
  MMDB_lookup_result_s *result = caml_stat_alloc(sizeof(*result));
  *result = MMDB_lookup_string(as_mmdb, as_string, &gai_error, &mmdb_error);
  check_error(gai_error, mmdb_error);
  free(as_string);

  while (iter_count != Val_emptylist) {
    total_len++;
    iter_count = Field(iter_count, 1);
  }

  char **query = caml_stat_alloc(sizeof(char *) * (total_len + 1));

  while (query_list != Val_emptylist) {
    query[copy_count] = caml_strdup(String_val(Field(query_list, 0)));
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
  free(result);
  for (int i = 0; i < copy_count; free(query[i]), i++);
  free(query);
  query_r = caml_alloc(2, 0);

  switch (entry_data.type) {
  case MMDB_DATA_TYPE_BYTES:
    clean_result = caml_stat_alloc(entry_data.data_size + 1);
    memcpy(clean_result, entry_data.bytes, entry_data.data_size);
    goto string_finish;

  case MMDB_DATA_TYPE_UTF8_STRING:
    clean_result = strndup(entry_data.utf8_string, entry_data.data_size);
    goto string_finish;

  case MMDB_DATA_TYPE_FLOAT:
    Store_field(query_r, 0, polymorphic_variants.poly_float);
    Store_field(query_r, 1, caml_copy_double(entry_data.float_value));
    goto finish;

  case MMDB_DATA_TYPE_BOOLEAN:
    Store_field(query_r, 0, polymorphic_variants.poly_bool);
    Store_field(query_r, 1, Val_true ? entry_data.boolean : Val_false);
    goto finish;

  case MMDB_DATA_TYPE_DOUBLE:
    Store_field(query_r, 0, polymorphic_variants.poly_float);
    Store_field(query_r, 1, caml_copy_double(entry_data.double_value));
    goto finish;

  case MMDB_DATA_TYPE_UINT16:
    Store_field(query_r, 0, polymorphic_variants.poly_int);
    int_result = Val_long(entry_data.uint16);
    goto int_finish;

  case MMDB_DATA_TYPE_UINT32:
    Store_field(query_r, 0, polymorphic_variants.poly_int);
    int_result = Val_long(entry_data.uint32);
    goto int_finish;

  case MMDB_DATA_TYPE_UINT64:
    Store_field(query_r, 0, polymorphic_variants.poly_int);
    int_result = Val_long(entry_data.uint32);
    goto int_finish;

    // look at /usr/bin/sed -n 1380,1430p src/maxminddb.c
  case MMDB_DATA_TYPE_ARRAY:
  case MMDB_DATA_TYPE_MAP:
    caml_failwith("Can't return a Map or Array yet");
  }

 string_finish:
  caml_clean_result = caml_copy_string(clean_result);
  free(clean_result);
  Store_field(query_r, 0, polymorphic_variants.poly_string);
  Store_field(query_r, 1, caml_clean_result);
  CAMLreturn(query_r);

 int_finish:
  Store_field(query_r, 1, int_result);
  CAMLreturn(query_r);

 finish:
  CAMLreturn(query_r);
}
