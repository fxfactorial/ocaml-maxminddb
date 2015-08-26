#require "maxminddb"

let city = "etc/GeoLite2-City.mmdb"
let country = "etc/GeoLite2-Country.mmdb"

let raw = "172.56.31.240"

let () =
  Maxminddb.with_mmdb city begin fun this_mmdb ->
    [Maxminddb.version ();
     Maxminddb.postal_code raw this_mmdb;
     Maxminddb.dump this_mmdb;
     Maxminddb.dump ~ip:raw this_mmdb]
    |> List.iter print_endline
  end
