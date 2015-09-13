let (city, country) = "etc/GeoLite2-City.mmdb", "etc/GeoLite2-Country.mmdb"

let (via_phone, some_ip) = "172.56.31.240", "69.12.169.82"

let () =
  (* Maxminddb.with_mmdb city begin fun this_mmdb -> *)
  let this_mmdb = Maxminddb.create city in
    [Maxminddb.version ();
     (match Maxminddb.lookup_path some_ip
              ["subdivisions"; "0"; "geoname_id"] this_mmdb with
      | `Int i -> string_of_int i
      | _ -> assert false);
     Maxminddb.postal_code some_ip this_mmdb;
     Maxminddb.dump this_mmdb;
     Maxminddb.dump ~ip:some_ip this_mmdb;
     Maxminddb.city_name ~lang:Maxminddb.Japanese ~ip:some_ip this_mmdb
    ]
    |> List.iter print_endline;
    let loc = Maxminddb.location some_ip this_mmdb in
    let open Maxminddb in
    Printf.sprintf
      "%f %f %d %s" loc.latitude loc.longitude loc.metro_code loc.time_zone
    |> print_endline;
    Maxminddb.country_name ~lang:Maxminddb.Russian ~ip:some_ip this_mmdb
    |> print_endline;
    Maxminddb.continent_name ~lang:Maxminddb.German ~ip:some_ip this_mmdb
    |> print_endline;
    Maxminddb.iso_code ~ip:some_ip this_mmdb
    |> print_endline;
    let all_together_now = Maxminddb.borders ~lang:Japanese ~ip:some_ip this_mmdb in
    Printf.sprintf
      "%s %s %s %s %s"
      all_together_now.postal_code
      all_together_now.city_name
      all_together_now.country_name
      all_together_now.continent_name
      all_together_now.iso_code
    |> print_endline;
    Gc.full_major ()
