#define SECRET
#define THINGNAME "hub"

const char WIFI_SSID[] = "CDTT";
const char WIFI_PASSWORD[] = "113114115";
const char AWS_IOT_ENDPOINT[] = "a2hoy94astii3x-ats.iot.ap-southeast-2.amazonaws.com";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
............copy your key here.............
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
............copy your key here.............
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
............copy your key here.............
-----END RSA PRIVATE KEY-----
)KEY";