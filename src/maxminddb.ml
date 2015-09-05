type unsafe_mmdb_handle

type mmdb = { mutable initialized : bool;
              handle : unsafe_mmdb_handle; }

type location = { latitude : float;
                  longitude: float;
                  metro_code : int;
                  time_zone : string; }

type borders = { postal_code : string;
                 city_name : string;
                 country_name : string;
                 continent_name : string;
                 iso_code : string; }

type languages =
  | German
  | English
  | Spanish
  | French
  | Japanese
  | Brazilian
  | Russian
  | Chinese

let string_of_language = function
  | German -> "de"
  | English -> "en"
  | Spanish -> "es"
  | French -> "fr"
  | Japanese -> "ja"
  | Brazilian -> "pt-BR"
  | Russian -> "ru"
  | Chinese -> "zh-CN"

type query_r = [`String of string
               | `Int of int
               | `Float of float
               | `Bool of bool]

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
  match lookup_path ip ["postal"; "code"] mmdb with
  | `String s -> s
  | _ -> assert false

let city_name ?(lang=English) ~ip mmdb =
  match lookup_path ip ["city"; "names"; string_of_language lang] mmdb with
  | `String s -> s
  | _ -> assert false

let country_name ?(lang=English) ~ip mmdb =
  match lookup_path ip ["country"; "names"; string_of_language lang] mmdb with
  | `String s -> s
  | _ -> assert false

let continent_name ?(lang=English) ~ip mmdb =
  match lookup_path ip ["continent"; "names"; string_of_language lang] mmdb with
  | `String s -> s
  | _ -> assert false

let iso_code ~ip mmdb =
  match lookup_path ip ["country"; "iso_code"] mmdb with
  | `String s -> s
  | _ -> assert false

let location ~ip mmdb =
  {latitude = (match lookup_path ip ["location"; "latitude"] mmdb with
       | `Float f -> f | _ -> assert false);
   longitude = (match lookup_path ip ["location"; "longitude"] mmdb with
       | `Float f -> f | _ -> assert false);
   metro_code = (match lookup_path ip ["location"; "metro_code"] mmdb with
       | `Int i -> i | _ -> assert false);
   time_zone = (match lookup_path ip ["location"; "time_zone"] mmdb with
       | `String s -> s | _ -> assert false)}

let borders ?(lang=English) ~ip mmdb =
  { postal_code = postal_code ip mmdb;
    city_name = city_name ~lang ~ip mmdb;
    country_name = country_name ~lang ~ip mmdb;
    continent_name = continent_name ~lang ~ip mmdb;
    iso_code = iso_code ip mmdb; }

let with_mmdb ~path f =
  let this_mmdb = create path in
  let () = f this_mmdb in
  close this_mmdb

