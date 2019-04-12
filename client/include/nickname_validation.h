// nickname_validation.h - Functionality to validate chat nicknames
//

#ifndef __NICKNAME_VALIDATION_H__
#define __NICKNAME_VALIDATION_H__

/**
 * @brief Checks whether a given nickname, i.e., chat handle, is valid.
 * @param getLineResult Result from calling GetLineFromUser.
 * @param pszNickname Pointer to buffer containing nickname to validate.
 * @return TRUE if the nickname matches the validation criteria; FALSE other-
 * wise.
 * @remarks Nicknames are subject to the following validation criteria: (1)
 * they are allowed to only be MAX_NICKNAME_LEN characters long; (2) they can
 * not contain spaces or special characters; and (3) they can contain upper
 * and lower case letters and/or numbers.
 */
BOOL IsNicknameValid(int getLineResult, const char* pszNickname);

#endif /* __NICKNAME_VALIDATION_H__ */
