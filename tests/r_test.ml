#require "maxminddb"

let city = "etc/GeoLite2-City.mmdb"
let country = "etc/GeoLite2-Country.mmdb"

let (via_phone, san_fran) = "172.56.31.240", "69.12.169.82"

let () =
  Maxminddb.with_mmdb city begin fun this_mmdb ->
    [Maxminddb.version ();
     Maxminddb.lookup_path san_fran ["subdivisions";"0";"geoname_id"] this_mmdb;
     Maxminddb.postal_code san_fran this_mmdb;
     Maxminddb.dump this_mmdb;
     Maxminddb.dump ~ip:san_fran this_mmdb
    ]
    |> List.iter print_endline
  end
