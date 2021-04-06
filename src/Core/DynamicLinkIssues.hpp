#pragma once
#include <ers/Issue.h>

ERS_DECLARE_ISSUE(core, DynamicLinkIssue, "Dynamic library name: " << dlName,
                  ((const char *)dlName))

ERS_DECLARE_ISSUE_BASE(core, CannotResolveModule, core::DynamicLinkIssue,
                       "Failed to resolve module - Reason: " << reason << " - ",
                       ((const char *)dlName), ((const char *)reason))

ERS_DECLARE_ISSUE_BASE(core, CannotOpenModule, core::DynamicLinkIssue,
                       "Failed to dlopen module - Reason: " << reason << " - ",
                       ((const char *)dlName), ((const char *)reason))
ERS_DECLARE_ISSUE(core, MissingCreateOrDelete, "Failed to resolve create and/or delete", ERS_EMPTY)