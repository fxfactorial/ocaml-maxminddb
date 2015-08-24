Maxminddb is an OCaml binding to [libmaxminddb](https://github.com/maxmind/libmaxminddb),
the successor of [GeoIP Legacy (Previously known as
GeoIP)](http://dev.maxmind.com/geoip/).

# Examples

I assume you have `libmaxminddb` already installed on your system.

```ocaml
(* This file is named dump_stats.ml *)
#require "maxminddb"

let () =
  Maxminddb.create "etc/GeoLite2-City.mmdb"
  |> Maxminddb.dump Sys.argv.(1) |> print_endline;
  Maxminddb.close t
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
          }
      }
    "continent": 
      {
        "code": 
          "NA" <utf8_string>
        "geoname_id": 
          6255149 <uint32>
.
.
```

Here's an even shorter example that uses a convenience function that
closes the `mmdb` handle for you.

```ocaml
(* File named zip_of_ip.ml *)
#require "maxminddb"
let () = 
  Maxminddb.with_mmdb "etc/GeoLite2-City.mmdb" begin fun this_mmdb ->
    Maxminddb.postal_code "172.56.31.240" this_mmdb |> print_endline;
  end
```

And the corresponding result on the shell

```ocaml
$ utop zip_of_ip.ml
90221
```
