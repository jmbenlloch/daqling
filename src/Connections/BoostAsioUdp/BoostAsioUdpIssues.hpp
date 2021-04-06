#include <ers/Issue.h>

namespace daqling {
ERS_DECLARE_ISSUE(connection, BoostAsioUdpIssue, "", ERS_EMPTY)
ERS_DECLARE_ISSUE_BASE(connection, ReceiveFailed, connection::BoostAsioUdpIssue,
                       "receive() failed cause: " << what, ERS_EMPTY, ((const char *)what))

ERS_DECLARE_ISSUE_BASE(connection, SendFailed, connection::BoostAsioUdpIssue,
                       "send() failed cause: " << what, ERS_EMPTY, ((const char *)what))

ERS_DECLARE_ISSUE_BASE(connection, InvalidTransportType, connection::BoostAsioUdpIssue,
                       "Unrecognized transport type: " << what, ERS_EMPTY, ((const char *)what))
ERS_DECLARE_ISSUE_BASE(connection, CannotAddChannel, connection::BoostAsioUdpIssue,
                       "Failed to add channel! returned: " << eWhat, ERS_EMPTY,
                       ((const char *)eWhat))
} // namespace daqling