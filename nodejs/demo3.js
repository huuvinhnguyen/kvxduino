
const coap  = require('coap') // or coap
    , req   = coap.request('coap://192.168.1.94/light')
var payload = "0";
req.write("0");

req.on('response', function(res) {
  res.pipe(process.stdout)
})

req.end()
