type mmdb

external version : unit -> string = "mmdb_ml_version"

external create : path:string -> mmdb = "mmdb_ml_open"

external close : mmdb -> unit = "mmdb_ml_close"

external dump : ip:string -> mmdb -> string =
  "mmdb_ml_dump"

external lookup_path : ip:string -> query:string list -> mmdb -> string =
  "mmdb_ml_lookup_path"

let postal_code ~ip mmdb =
  lookup_path ip ["postal";"code"] mmdb

let with_mmdb ~path f =
  let this_mmdb = create path in
  f this_mmdb |> ignore;
  close this_mmdb
