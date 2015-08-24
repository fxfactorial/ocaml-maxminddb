type mmdb
type lookup_result

external version : unit -> string = "mmdb_ml_version"
external init : path:string -> mmdb = "mmdb_ml_open"
external close : mmdb -> unit = "mmdb_ml_close"
external dump_lookup : ip:string -> mmdb -> string = "mmdb_ml_lookup_string"
