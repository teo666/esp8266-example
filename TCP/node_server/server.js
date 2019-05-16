const net = require('net');

let server = net.createServer( (s) => {
    s.on("data", (data)=>{
        console.log(data.toString('utf8'));
    });
});

server.on("connection", (evt) =>{
    console.log("connessione avvenuta");
})

server.listen(2000, "192.168.0.12", 1, function(e){
    console.log("server in ascolto");
})