// nickname_validation.c - Functionality to determine if chat handles/names that
// the user wants are valid.
//

#include "stdafx.h"
#include "client.h"

#include "client_functions.h"
#include "nickname_validation.h"

///////////////////////////////////////////////////////////////////////////////
// IsNicknameValid function - Determines if a value for the nickname that the
// user wants meets validation criteria
//

BOOL IsNicknameValid(int getLineResult, const char* pszNickname) {
	if (getLineResult != OK) {
		if (TOO_LONG == getLineResult) {
			fprintf(stderr, NICKNAME_TOOLONG, MAX_NICKNAME_LEN);
		} else {
			fprintf(stderr, NICKNAME_UNKERROR);
		}
		return FALSE;
	}

	if (IsNullOrWhiteSpace(pszNickname)) {
		fprintf(stderr, NICKNAME_REQUIRED);
		return FALSE;
	}

	if (!IsAlphaNumeric(pszNickname)) {
		fprintf(stderr, NICKNAME_NOTALPHA);
		return FALSE;
	}

	// Nickname meets all validation criteria
	return TRUE;
}
