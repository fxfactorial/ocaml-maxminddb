Maxminddb is an OCaml binding to [libmaxminddb](https://github.com/maxmind/libmaxminddb),
the successor of [GeoIP Legacy (Previously known as
GeoIP)](http://dev.maxmind.com/geoip/).

# Example

Once you have `maxminddb` installed, try out the following example

```ocaml
(* This file is named dump_stats.ml *)
#require "maxminddb"
let city = "etc/GeoLite2-City.mmdb"
let () =
  let t = Maxminddb.init city in
  Maxminddb.dump_lookup Sys.argv.(1) t |> print_endline;
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
            "en": 
              "Compton" <utf8_string>
            "fr": 
              "Compton" <utf8_string>
            "ja": 
              "コンプトン" <utf8_string>
            "ru": 
              "Комптон" <utf8_string>
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
        "names": 
.
.
.
```
