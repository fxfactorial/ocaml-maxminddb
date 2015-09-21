Maxminddb is an OCaml binding to [libmaxminddb](https://github.com/maxmind/libmaxminddb),
the successor of [GeoIP Legacy (Previously known as
GeoIP)](http://dev.maxmind.com/geoip/).

Lookups return polymorphic variants wrapping values like `bool`,
`float`, `string` and `int`.
install with

```shell
opam install maxminddb
```

# Code Examples

I assume you have `libmaxminddb` already installed on your system.

Here's a short example of what you can do with the library.

```ocaml
(* File named loc_dump.ml *)
#require "maxminddb"

let () =
  let some_ip = "172.56.31.240" in
  let this_mmdb = Maxminddb.create "etc/GeoLite2-City.mmdb" in
  let loc = Maxminddb.location some_ip this_mmdb in
  let open Maxminddb in
  Printf.sprintf "%f %f %d %s" loc.latitude loc.longitude loc.metro_code loc.time_zone
  |> print_endline;
  let geo_borders = borders ~lang:French ~ip:some_ip this_mmdb in
  Printf.sprintf
    "%s %s %s %s %s"
    geo_borders.postal_code
    geo_borders.city_name
    geo_borders.country_name
    geo_borders.continent_name
    geo_borders.iso_code
  |> print_endline
```

And the corresponding result on the shell

```shell
$ utop loc_dump.ml
33.895900 -118.220100 803 America/Los_Angeles
90221 Compton États-Unis Amérique du Nord US
```

Here's dumping example

```ocaml
(* This file is named dump_stats.ml *)
#require "maxminddb"

let () =
  Maxminddb.create "etc/GeoLite2-City.mmdb"
  |> Maxminddb.dump Sys.argv.(1) |> print_endline
```

And at the shell

```shell
$ utop dump_stats.ml "172.56.31.240"
{
    "city": 
      {
        "geoname_id": 
          5339066 <uint32>
        "names": 
          {
            "de": 
              "Compton" <utf8_string>
            .
            .
            "zh-CN": 
              "康普顿" <utf8_string>
.
.
```

Also note that you can query the IP database by array indexing as
well.

```ocaml
(* This file is named dump_using_array.ml *)
#require "maxminddb"

let () =
  let some_ip = "69.12.169.82" in
  let this_mmdb = Maxminddb.create "etc/GeoLite2-City.mmdb" in
  match Maxminddb.lookup_path some_ip ["subdivisions";"0";"geoname_id"] this_mmdb with
  | `Int i -> string_of_int i |> print_endline
  | _ -> assert false
```

```shell
$ utop dump_using_array.ml
5332921
```

# Issues

1.  If your path yields a map or array then the library will throw an
    exception, this is mostly an implementation issue. I haven't
    thought about how I want to do it yet.
