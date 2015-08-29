Maxminddb is an OCaml binding to [libmaxminddb](https://github.com/maxmind/libmaxminddb),
the successor of [GeoIP Legacy (Previously known as
GeoIP)](http://dev.maxmind.com/geoip/).

Lookups return polymorphic variants wrapping values like `bool`,
`float`, `string` and `int`.

# Code Examples

I assume you have `libmaxminddb` already installed on your system.

Here's a short example that uses a convenience function that
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

Here's a slightly longer example

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
.
.
```

Also note that you can query the IP database by array indexing as
well.

```ocaml
(* This file is named dump_using_array.ml *)
#require "maxminddb"

let () =
  let san_fran = "69.12.169.82" in
  Maxminddb.with_mmdb "etc/GeoLite2-City.mmdb" begin fun this_mmdb ->
    match Maxminddb.lookup_path san_fran ["subdivisions";"0";"geoname_id"] this_mmdb with
    | `Int i -> string_of_int i |> print_endline
    | _ -> assert false
  end
```

```shell
$ utop dump_using_array.ml
5332921
```

# Issues

1.  If your path yields a map or array then the library will throw an
    exception, this is mostly an implementation issue. I haven't
    thought about how I want to do it yet.
2.  Sometimes there's junk at the end of string for a DB dump. This is
    because of the awkward C API where the dump function expects a
    `FILE *` and I don't have access to [fmemopen](http://pubs.opengroup.org/onlinepubs/9699919799/functions/fmemopen.html) since I am
    primarily developing this library on `OS X`.
