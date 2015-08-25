type mmdb

external version_raw : unit -> string = "mmdb_ml_version"

external create : path:string -> mmdb = "mmdb_ml_open"

external close : mmdb -> unit = "mmdb_ml_close"

external dump_raw : ip:string -> mmdb -> string =
  "mmdb_ml_dump"

external lookup_path_raw : ip:string -> query:string list -> mmdb -> string =
  "mmdb_ml_lookup_path"

let without_null s =
  String.sub s 0 (String.length s - 1)

let dump ~ip mmdb =
  dump_raw ~ip mmdb |> without_null

let version () =
  version_raw () |> without_null

let lookup_path ~ip ~query mmdb =
  lookup_path_raw ~ip ~query mmdb |> without_null

let postal_code ~ip mmdb =
  lookup_path ip ["postal";"code"] mmdb

let with_mmdb ~path f =
  let this_mmdb = create path in
  f this_mmdb |> ignore;
  close this_mmdb

(* let address_of_zip zip = *)
(*   let query = *)
(*     Printf.sprintf "http://maps.googleapis.com/maps/api/geocode/json?address=%s&sensor=false" zip *)
(*   in *)
(*   Podge.Web.get query *)
(*   |> Yojson.Basic.Util.member "body" *)
(*   (\* |> Podge.Yojson.show_pretty_of_in_mem *\) *)
(*   |> Yojson.Basic.Util.member "results" *)
(*   (\* |> Yojson.Basic.Util.index 0 *\) *)
(*   |> Yojson.Basic.Util.member "formatted_address" *)
(*   |> Yojson.Basic.to_string *)
