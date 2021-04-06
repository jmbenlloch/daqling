#include <ers/Issue.h>

namespace daqling {
ERS_DECLARE_ISSUE(connection, ZMQIssue, "", ERS_EMPTY)
ERS_DECLARE_ISSUE_BASE(connection, ReceiveFailed, connection::ZMQIssue,
                       "ZMQ recv() failed cause: " << what, ERS_EMPTY, ((const char *)what))

ERS_DECLARE_ISSUE_BASE(connection, SendFailed, connection::ZMQIssue,
                       "ZMQ send() failed cause: " << what, ERS_EMPTY, ((const char *)what))

ERS_DECLARE_ISSUE_BASE(connection, InvalidTransportType, connection::ZMQIssue,
                       "Unrecognized transport type: " << what, ERS_EMPTY, ((const char *)what))
ERS_DECLARE_ISSUE_BASE(connection, CannotAddChannel, connection::ZMQIssue,
                       "Failed to add channel! ZMQ returned: " << eWhat, ERS_EMPTY,
                       ((const char *)eWhat))
} // namespace daqling