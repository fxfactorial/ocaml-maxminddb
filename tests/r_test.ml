#require "maxminddb, podge"

let city = "etc/GeoLite2-City.mmdb"
let country = "etc/GeoLite2-Country.mmdb"

let raw = "172.56.31.240"

let () =
  let t = Maxminddb.init city in
  Podge.ANSITerminal.colored_message "Opened DB" |> Podge.Printf.printfn "%s";
  Maxminddb.dump_lookup raw t |> print_endline;
  Maxminddb.close t;
  Podge.ANSITerminal.colored_message "Closed DB" |> Podge.Printf.printfn "%s";
