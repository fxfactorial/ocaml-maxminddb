type unsafe_mmdb_handle

type mmdb = { mutable initialized : bool;
              handle : unsafe_mmdb_handle; }

type query_r = [`String of string | `Int of int | `Float of float | `Bool of bool]

external version : unit -> string = "mmdb_ml_version"

external create : path:string -> mmdb = "mmdb_ml_open"

external close : mmdb -> unit = "mmdb_ml_close"

external dump_per_ip_raw : string -> mmdb -> string =
  "mmdb_ml_dump_per_ip"

external dump_global_raw : mmdb -> string =
  "mmdb_ml_dump_global"

external lookup_path_raw : ip:string -> query:string list -> mmdb -> query_r =
  "mmdb_ml_lookup_path"

let without_null s =
  String.sub s 0 (String.length s - 1)

let dump ?ip mmdb =
  (match ip with
   | None -> dump_global_raw mmdb
   | Some ip -> dump_per_ip_raw ip mmdb)
  |> without_null

let lookup_path ~ip ~query mmdb =
  match query with
  | [] -> raise (Invalid_argument "Must provide non empty query path")
  | _ -> lookup_path_raw ~ip ~query mmdb

let postal_code ~ip mmdb =
  match lookup_path ip ["postal";"code"] mmdb with
  | `String s -> s
  | x ->
    assert false

let with_mmdb ~path f =
  let this_mmdb = create path in
  let () = f this_mmdb in
  close this_mmdb
