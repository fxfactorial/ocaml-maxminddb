(** Maxmind is a binding to libmaxminddb with some higher level
    functionality *)

type mmdb

(** Get version string of Maxminddb *)
val version : unit -> string

(** Create a handle on a mmdb descriptor based off a .mmdb database
    file. Maxminddb comes with City, Country databases in etc *)
val create : path:string -> mmdb

(** Close a handle on a mmdb descriptor, bad things will happen if you
    try to use the handle after you called closed on it. *)
val close : mmdb -> unit

(** Dumps the database as a string, do not depend on this structure:
    it is not JSON, just looks like it. *)
val dump : ip:string -> mmdb -> string

(** For a given ip address, think 127.0.0.1, query path and mmdb
    handle, get the result. Note path must be safe, look at dump
    first *)
val lookup_path : ip:string -> query:string list -> mmdb -> string

(** Short cut function for getting postal code from ip address *)
val postal_code : ip:string -> mmdb -> string

(** Convenience function that opens and closes a mmdb for you *)
val with_mmdb : path:string -> (mmdb -> 'a) -> unit
