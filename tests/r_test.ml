let (city, country) = "etc/GeoLite2-City.mmdb", "etc/GeoLite2-Country.mmdb"

let (via_phone, san_fran) = "172.56.31.240", "69.12.169.82"

let () =
  Maxminddb.with_mmdb city begin fun this_mmdb ->
    [Maxminddb.version ();
     (match Maxminddb.lookup_path san_fran ["subdivisions";"0";"geoname_id"] this_mmdb with
      | `Int i -> string_of_int i
      | _ -> assert false);
     Maxminddb.postal_code san_fran this_mmdb;
     Maxminddb.dump this_mmdb;
     Maxminddb.dump ~ip:san_fran this_mmdb;
     Maxminddb.city_name ~lang:Maxminddb.Japanese ~ip:san_fran this_mmdb
    ]
    |> List.iter print_endline
  end
