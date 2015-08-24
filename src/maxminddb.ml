type mmdb

type lookup_result

external version : unit -> string = "mmdb_ml_version"

external init : path:string -> mmdb = "mmdb_ml_open"

external close : mmdb -> unit = "mmdb_ml_close"

(** Dumps the database as a string, do not depend on this structure:
    it is not JSON, just looks like it. *)

external query_result : ip:string -> mmdb -> lookup_result = "mmdb_ml_pull_result"

external dump : ip:string -> mmdb -> string =
  "mmdb_ml_dump"

external lookup_path : ip:string -> query:string list -> mmdb -> string =
  "mmdb_ml_lookup_path"
