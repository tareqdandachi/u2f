#pragma mark - packet helpers

// PACKETS ARE DEFINED AS INITIAL AND CONTINUATION WHERE

// INIT PACKET:
// 4 for channel id
// 1 for command identifier
// 1 for BCNTH
// 1 for BCNTL
// 57 for payload

// CONT PACKET:
// 4 for channel id
// 1 for packet sequence
// 59 for payload

// we can calculate this from max packet size of 64 bytes for
// full-speed devices, so payload size is at max
// 64 - 7 + 128 * (64 - 5) = 7609 bytes.
#define MAX_PACKET_LENGTH 7609

// initial packet size is 64 - 7
#define MAX_PACKET_LENGTH_INIT 57

// continuation packet size is 64 - 5
#define MAX_PACKET_LENGTH_CONT 59

#define IS_CONTINUATION_PACKET(x) ( (x) < 0x80)
#define IS_NOT_CONTINUATION_PACKET(x) ( (x) >= 0x80)

#define SET_MSG_LEN(b, v) do { \
	(b)[5] = ((v) >> 8) & 0xff;  (b)[6] = (v) & 0xff; \
} while(0)

#define PACKET_DELAY_US 2500

#pragma mark - u2f hid transport headers from
// https://fidoalliance.org/specs/fido-u2f-v1.2-ps-20170411/inc/u2f_hid.h
// TODO: move these into a header file later...

	// Size of HID reports

	#define HID_RPT_SIZE            64      // Default size of raw HID report

	// Frame layout - command- and continuation frames

	#define CID_BROADCAST           0xffffffff // Broadcast channel id

	#define TYPE_MASK               0x80    // Frame type mask
	#define TYPE_INIT               0x80    // Initial frame identifier
	#define TYPE_CONT               0x00    // Continuation frame identifier

	// typedef struct {
	//   uint32_t cid;                        // Channel identifier
	//   union {
	//     uint8_t type;                      // Frame type - b7 defines type
	//     struct {
	//       uint8_t cmd;                     // Command - b7 set
	//       uint8_t bcnth;                   // Message byte count - high part
	//       uint8_t bcntl;                   // Message byte count - low part
	//       uint8_t data[HID_RPT_SIZE - 7];  // Data payload
	//     } init;
	//     struct {
	//       uint8_t seq;                     // Sequence number - b7 cleared
	//       uint8_t data[HID_RPT_SIZE - 5];  // Data payload
	//     } cont;
	//   };
	// } U2FHID_FRAME;

	#define FRAME_TYPE(f) ((f).type & TYPE_MASK)
	#define FRAME_CMD(f)  ((f).init.cmd & ~TYPE_MASK)
	#define MSG_LEN(f)    ((f).init.bcnth*256 + (f).init.bcntl)
	#define FRAME_SEQ(f)  ((f).cont.seq & ~TYPE_MASK)

	// HID usage- and usage-page definitions

	#define FIDO_USAGE_PAGE         0xf1d0  // FIDO alliance HID usage page
	#define FIDO_USAGE_U2FHID       0x01    // U2FHID usage for top-level collection
	#define FIDO_USAGE_DATA_IN      0x20    // Raw IN data report
	#define FIDO_USAGE_DATA_OUT     0x21    // Raw OUT data report

	// General constants

	#define U2FHID_IF_VERSION       2       // Current interface implementation version
	#define U2FHID_TRANS_TIMEOUT    3000    // Default message timeout in ms

	// U2FHID native commands

	#define U2FHID_PING         (TYPE_INIT | 0x01)  // Echo data through local processor only
	#define U2FHID_MSG          (TYPE_INIT | 0x03)  // Send U2F message frame
	#define U2FHID_LOCK         (TYPE_INIT | 0x04)  // Send lock channel command
	#define U2FHID_INIT         (TYPE_INIT | 0x06)  // Channel initialization
	#define U2FHID_WINK         (TYPE_INIT | 0x08)  // Send device identification wink
	#define U2FHID_SYNC         (TYPE_INIT | 0x3c)  // Protocol resync command
	#define U2FHID_ERROR        (TYPE_INIT | 0x3f)  // Error response

	#define U2FHID_VENDOR_FIRST (TYPE_INIT | 0x40)  // First vendor defined command
	#define U2FHID_VENDOR_LAST  (TYPE_INIT | 0x7f)  // Last vendor defined command

	// U2FHID_INIT command defines

	#define INIT_NONCE_SIZE         8       // Size of channel initialization challenge
	#define CAPFLAG_WINK            0x01    // Device supports WINK command

	typedef struct {
	  uint8_t nonce[INIT_NONCE_SIZE];       // Client application nonce
	} U2FHID_INIT_REQ;

	typedef struct {
	  uint8_t nonce[INIT_NONCE_SIZE];       // Client application nonce
	  uint32_t cid;                         // Channel identifier
	  uint8_t versionInterface;             // Interface version
	  uint8_t versionMajor;                 // Major version number
	  uint8_t versionMinor;                 // Minor version number
	  uint8_t versionBuild;                 // Build version number
	  uint8_t capFlags;                     // Capabilities flags
	} U2FHID_INIT_RESP;

	// U2FHID_SYNC command defines

	typedef struct {
	  uint8_t nonce;                        // Client application nonce
	} U2FHID_SYNC_REQ;

	typedef struct {
	  uint8_t nonce;                        // Client application nonce
	} U2FHID_SYNC_RESP;

	// Low-level error codes. Return as negatives.

	#define ERR_NONE                0x00    // No error
	#define ERR_INVALID_CMD         0x01    // Invalid command
	#define ERR_INVALID_PAR         0x02    // Invalid parameter
	#define ERR_INVALID_LEN         0x03    // Invalid message length
	#define ERR_INVALID_SEQ         0x04    // Invalid message sequencing
	#define ERR_MSG_TIMEOUT         0x05    // Message has timed out
	#define ERR_CHANNEL_BUSY        0x06    // Channel busy
	#define ERR_LOCK_REQUIRED       0x0a    // Command requires channel lock
	#define ERR_SYNC_FAIL           0x0b    // SYNC command failed
	#define ERR_OTHER               0x7f    // Other unspecified error

#pragma mark - u2f raw message format header from
// https://fidoalliance.org/specs/fido-u2f-v1.2-ps-20170411/inc/u2f.h
// TODO: move into a separate header file too

	// U2F native commands

	#define U2F_REGISTER            0x01    // Registration command
	#define U2F_AUTHENTICATE        0x02    // Authenticate/sign command
	#define U2F_VERSION             0x03    // Read version string command

	#define U2F_VENDOR_FIRST        0x40    // First vendor defined command
	#define U2F_VENDOR_LAST         0xbf    // Last vendor defined command

	// U2F_CMD_REGISTER command defines

	#define U2F_REGISTER_ID         0x05    // Version 2 registration identifier
	#define U2F_REGISTER_HASH_ID    0x00    // Version 2 hash identintifier

	// Authentication control byte

	#define U2F_AUTH_ENFORCE        0x03    // Enforce user presence and sign
	#define U2F_AUTH_CHECK_ONLY     0x07    // Check only
	#define U2F_AUTH_FLAG_TUP       0x01    // Test of user presence set

	// Command status responses

	#define U2F_SW_NO_ERROR                 0x9000 // SW_NO_ERROR
	#define U2F_SW_WRONG_DATA               0x6A80 // SW_WRONG_DATA
	#define U2F_SW_CONDITIONS_NOT_SATISFIED 0x6985 // SW_CONDITIONS_NOT_SATISFIED
	#define U2F_SW_COMMAND_NOT_ALLOWED      0x6986 // SW_COMMAND_NOT_ALLOWED
	#define U2F_SW_INS_NOT_SUPPORTED        0x6D00 // SW_INS_NOT_SUPPORTED

#pragma mark - Command status responses
// Sourced from ISO-7816
	#define SW_NO_ERROR                       0x9000
	#define SW_CONDITIONS_NOT_SATISFIED       0x6985
	#define SW_WRONG_DATA                     0x6A80
	#define SW_WRONG_LENGTH                   0x6700
	#define SW_INS_NOT_SUPPORTED              0x6D00
	#define SW_CLA_NOT_SUPPORTED              0x6E00

#define ADD_SW_OK(x) do { (*x++)=0x90; (*x++)=0x00;} while (0)

#define ADD_SW_COND(x) \
    do                 \
    {                  \
        (*x++) = 0x69; \
        (*x++) = 0x85; \
    } while (0)

#define ADD_SW_WRONG_DATA(x) \
    do                 \
    {                  \
        (*x++) = 0x6A; \
        (*x++) = 0x80; \
    } while (0)