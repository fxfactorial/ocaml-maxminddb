type mmdb

(** Get version string of Maxminddb *)
external version : unit -> string = "mmdb_ml_version"

(** Create a handle on a mmdb descriptor based off a .mmdb database
    file. Maxminddb comes with City, Country databases in etc *)
external create : path:string -> mmdb = "mmdb_ml_open"

(** Close a handle on a mmdb descriptor *)
external close : mmdb -> unit = "mmdb_ml_close"

(** Dumps the database as a string, do not depend on this structure:
    it is not JSON, just looks like it. *)
external dump : ip:string -> mmdb -> string =
  "mmdb_ml_dump"

(** For a given ip address, think 127.0.0.1, query path and mmdb
    handle, get the result. Note path must be safe, look at dump
    first *)
external lookup_path : ip:string -> query:string list -> mmdb -> string =
  "mmdb_ml_lookup_path"

(** Short cut function for getting postal code from ip address *)
val postal_code : ip:string -> mmdb -> string

(** Convenience function that opens and closes a mmdb for you *)
val with_mmdb : path:string -> (mmdb -> 'a) -> unit
