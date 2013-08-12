/** @file
Entry and initialization module for the browser.

Copyright (c) 2007 - 2013, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "FormDisplay.h"

//
// Search table for UiDisplayMenu()
//
SCAN_CODE_TO_SCREEN_OPERATION     gScanCodeToOperation[] = {
  {
    SCAN_UP,
    UiUp,
  },
  {
    SCAN_DOWN,
    UiDown,
  },
  {
    SCAN_PAGE_UP,
    UiPageUp,
  },
  {
    SCAN_PAGE_DOWN,
    UiPageDown,
  },
  {
    SCAN_ESC,
    UiReset,
  },
  {
    SCAN_LEFT,
    UiLeft,
  },
  {
    SCAN_RIGHT,
    UiRight,
  }
};

UINTN mScanCodeNumber = sizeof (gScanCodeToOperation) / sizeof (gScanCodeToOperation[0]);

SCREEN_OPERATION_T0_CONTROL_FLAG  gScreenOperationToControlFlag[] = {
  {
    UiNoOperation,
    CfUiNoOperation,
  },
  {
    UiSelect,
    CfUiSelect,
  },
  {
    UiUp,
    CfUiUp,
  },
  {
    UiDown,
    CfUiDown,
  },
  {
    UiLeft,
    CfUiLeft,
  },
  {
    UiRight,
    CfUiRight,
  },
  {
    UiReset,
    CfUiReset,
  },
  {
    UiPageUp,
    CfUiPageUp,
  },
  {
    UiPageDown,
    CfUiPageDown
  }, 
  {
    UiHotKey,
    CfUiHotKey
  }
};

EFI_GUID  gDisplayEngineGuid = {
  0xE38C1029, 0xE38F, 0x45b9, {0x8F, 0x0D, 0xE2, 0xE6, 0x0B, 0xC9, 0xB2, 0x62}
};

UINTN                         gSequence;
EFI_SCREEN_DESCRIPTOR         gStatementDimensions;
EFI_SCREEN_DESCRIPTOR         gOldStatementDimensions = {0};
BOOLEAN                       mStatementLayoutIsChanged = TRUE;
USER_INPUT                    *gUserInput;
FORM_DISPLAY_ENGINE_FORM      *gFormData;
EFI_HII_HANDLE                gHiiHandle;
UINT16                        gDirection;
LIST_ENTRY                    gMenuOption;
DISPLAY_HIGHLIGHT_MENU_INFO   gHighligthMenuInfo = {0};

//
// Browser Global Strings
//
CHAR16            *gFormNotFound;
CHAR16            *gNoSubmitIf;
CHAR16            *gBrwoserError;
CHAR16            *gSaveFailed;
CHAR16            *gPromptForData;
CHAR16            *gPromptForPassword;
CHAR16            *gPromptForNewPassword;
CHAR16            *gConfirmPassword;
CHAR16            *gConfirmError;
CHAR16            *gPassowordInvalid;
CHAR16            *gPressEnter;
CHAR16            *gEmptyString;
CHAR16            *gMiniString;
CHAR16            *gOptionMismatch;
CHAR16            *gFormSuppress;
CHAR16            *gProtocolNotFound;

CHAR16            gPromptBlockWidth;
CHAR16            gOptionBlockWidth;
CHAR16            gHelpBlockWidth;
CHAR16            *mUnknownString;

FORM_DISPLAY_DRIVER_PRIVATE_DATA  mPrivateData = {
  FORM_DISPLAY_DRIVER_SIGNATURE,
  NULL,
  {
    FormDisplay,
    ClearDisplayPage,
    ConfirmDataChange
  }
};


/**
  Get the string based on the StringId and HII Package List Handle.

  @param  Token                  The String's ID.
  @param  HiiHandle              The package list in the HII database to search for
                                 the specified string.

  @return The output string.

**/
CHAR16 *
GetToken (
  IN  EFI_STRING_ID                Token,
  IN  EFI_HII_HANDLE               HiiHandle
  )
{
  EFI_STRING  String;

  String = HiiGetString (HiiHandle, Token, NULL);
  if (String == NULL) {
    String = AllocateCopyPool (StrSize (mUnknownString), mUnknownString);
    ASSERT (String != NULL);
  }

  return (CHAR16 *) String;
}


/**
  Initialize the HII String Token to the correct values.

**/
VOID
InitializeDisplayStrings (
  VOID
  )
{
  mUnknownString        = GetToken (STRING_TOKEN (UNKNOWN_STRING), gHiiHandle);
  gSaveFailed           = GetToken (STRING_TOKEN (SAVE_FAILED), gHiiHandle);
  gPromptForData        = GetToken (STRING_TOKEN (PROMPT_FOR_DATA), gHiiHandle);
  gPromptForPassword    = GetToken (STRING_TOKEN (PROMPT_FOR_PASSWORD), gHiiHandle);
  gPromptForNewPassword = GetToken (STRING_TOKEN (PROMPT_FOR_NEW_PASSWORD), gHiiHandle);
  gConfirmPassword      = GetToken (STRING_TOKEN (CONFIRM_PASSWORD), gHiiHandle);
  gConfirmError         = GetToken (STRING_TOKEN (CONFIRM_ERROR), gHiiHandle);
  gPassowordInvalid     = GetToken (STRING_TOKEN (PASSWORD_INVALID), gHiiHandle);
  gPressEnter           = GetToken (STRING_TOKEN (PRESS_ENTER), gHiiHandle);
  gEmptyString          = GetToken (STRING_TOKEN (EMPTY_STRING), gHiiHandle);
  gMiniString           = GetToken (STRING_TOKEN (MINI_STRING), gHiiHandle);
  gOptionMismatch       = GetToken (STRING_TOKEN (OPTION_MISMATCH), gHiiHandle);
  gFormSuppress         = GetToken (STRING_TOKEN (FORM_SUPPRESSED), gHiiHandle);
  gProtocolNotFound     = GetToken (STRING_TOKEN (PROTOCOL_NOT_FOUND), gHiiHandle);
  gFormNotFound         = GetToken (STRING_TOKEN (STATUS_BROWSER_FORM_NOT_FOUND), gHiiHandle);
  gNoSubmitIf           = GetToken (STRING_TOKEN (STATUS_BROWSER_NO_SUBMIT_IF), gHiiHandle);
  gBrwoserError         = GetToken (STRING_TOKEN (STATUS_BROWSER_ERROR), gHiiHandle);
}

/**
  Free up the resource allocated for all strings required
  by Setup Browser.

**/
VOID
FreeDisplayStrings (
  VOID
  )
{
  FreePool (mUnknownString);
  FreePool (gEmptyString);
  FreePool (gSaveFailed);
  FreePool (gPromptForData);
  FreePool (gPromptForPassword);
  FreePool (gPromptForNewPassword);
  FreePool (gConfirmPassword);
  FreePool (gConfirmError);
  FreePool (gPassowordInvalid);
  FreePool (gPressEnter);
  FreePool (gMiniString);
  FreePool (gOptionMismatch);
  FreePool (gFormSuppress);
  FreePool (gProtocolNotFound);
  FreePool (gBrwoserError);
  FreePool (gNoSubmitIf);
  FreePool (gFormNotFound);
}

/**
  Get prompt string id from the opcode data buffer.

  @param  OpCode                 The input opcode buffer.

  @return The prompt string id.

**/
EFI_STRING_ID
GetPrompt (
  IN EFI_IFR_OP_HEADER     *OpCode
  )
{
  EFI_IFR_STATEMENT_HEADER  *Header;

  if (OpCode->Length <= sizeof (EFI_IFR_OP_HEADER)) {
    return 0;
  }

  Header = (EFI_IFR_STATEMENT_HEADER  *) (OpCode + 1);

  return Header->Prompt;
}

/**
  Get the supported width for a particular op-code

  @param  Statement              The curent statement.

  @return Returns the number of CHAR16 characters that is support.

**/
UINT16
GetWidth (
  IN FORM_DISPLAY_ENGINE_STATEMENT        *Statement
  )
{
  CHAR16       *String;
  UINTN        Size;
  UINT16       Width;
  EFI_IFR_TEXT *TestOp;

  Size = 0;

  //
  // See if the second text parameter is really NULL
  //
  if (Statement->OpCode->OpCode == EFI_IFR_TEXT_OP) {
    TestOp = (EFI_IFR_TEXT *) Statement->OpCode;
    if (TestOp->TextTwo != 0) {
      String = GetToken (TestOp->TextTwo, gFormData->HiiHandle);
      Size   = StrLen (String);
      FreePool (String);
    }
  }

  if ((Statement->OpCode->OpCode == EFI_IFR_SUBTITLE_OP) ||
      (Statement->OpCode->OpCode == EFI_IFR_REF_OP) ||
      (Statement->OpCode->OpCode == EFI_IFR_PASSWORD_OP) ||
      (Statement->OpCode->OpCode == EFI_IFR_ACTION_OP) ||
      (Statement->OpCode->OpCode == EFI_IFR_RESET_BUTTON_OP) ||
      //
      // Allow a wide display if text op-code and no secondary text op-code
      //
      ((Statement->OpCode->OpCode == EFI_IFR_TEXT_OP) && (Size == 0))
      ) {
    Width = (UINT16) (gPromptBlockWidth + gOptionBlockWidth);
  } else {
    Width = (UINT16) gPromptBlockWidth;
  }

  return (UINT16) (Width - LEFT_SKIPPED_COLUMNS);
}

/**
  Will copy LineWidth amount of a string in the OutputString buffer and return the
  number of CHAR16 characters that were copied into the OutputString buffer.
  The output string format is:
    Glyph Info + String info + '\0'.

  In the code, it deals \r,\n,\r\n same as \n\r, also it not process the \r or \g.

  @param  InputString            String description for this option.
  @param  LineWidth              Width of the desired string to extract in CHAR16
                                 characters
  @param  GlyphWidth             The glyph width of the begin of the char in the string.
  @param  Index                  Where in InputString to start the copy process
  @param  OutputString           Buffer to copy the string into

  @return Returns the number of CHAR16 characters that were copied into the OutputString 
  buffer, include extra glyph info and '\0' info.

**/
UINT16
GetLineByWidth (
  IN      CHAR16                      *InputString,
  IN      UINT16                      LineWidth,
  IN OUT  UINT16                      *GlyphWidth,
  IN OUT  UINTN                       *Index,
  OUT     CHAR16                      **OutputString
  )
{
  UINT16          StrOffset;
  UINT16          GlyphOffset;
  UINT16          OriginalGlyphWidth;
  BOOLEAN         ReturnFlag;
  UINT16          LastSpaceOffset;
  UINT16          LastGlyphWidth;

  if (InputString == NULL || Index == NULL || OutputString == NULL) {
    return 0;
  }

  if (LineWidth == 0 || *GlyphWidth == 0) {
    return 0;
  }

  //
  // Save original glyph width.
  //
  OriginalGlyphWidth = *GlyphWidth;
  LastGlyphWidth     = OriginalGlyphWidth;
  ReturnFlag         = FALSE;
  LastSpaceOffset    = 0;

  //
  // NARROW_CHAR can not be printed in screen, so if a line only contain  the two CHARs: 'NARROW_CHAR + CHAR_CARRIAGE_RETURN' , it is a empty line  in Screen.
  // To avoid displaying this  empty line in screen,  just skip  the two CHARs here.
  //
  if ((InputString[*Index] == NARROW_CHAR) && (InputString[*Index + 1] == CHAR_CARRIAGE_RETURN)) {
    *Index = *Index + 2;
  }

  //
  // Fast-forward the string and see if there is a carriage-return in the string
  //
  for (StrOffset = 0, GlyphOffset = 0; GlyphOffset <= LineWidth; StrOffset++) {
    switch (InputString[*Index + StrOffset]) {
      case NARROW_CHAR:
        *GlyphWidth = 1;
        break;

      case WIDE_CHAR:
        *GlyphWidth = 2;
        break;

      case CHAR_CARRIAGE_RETURN:
      case CHAR_LINEFEED:
      case CHAR_NULL:
        ReturnFlag = TRUE;
        break;

      default:
        GlyphOffset = GlyphOffset + *GlyphWidth;

        //
        // Record the last space info in this line. Will be used in rewind.
        //
        if ((InputString[*Index + StrOffset] == CHAR_SPACE) && (GlyphOffset <= LineWidth)) {
          LastSpaceOffset = StrOffset;
          LastGlyphWidth  = *GlyphWidth;
        }
        break;
    }

    if (ReturnFlag) {
      break;
    }
  } 

  //
  // Rewind the string from the maximum size until we see a space to break the line
  //
  if (GlyphOffset > LineWidth) {
    //
    // Rewind the string to last space char in this line.
    //
    if (LastSpaceOffset != 0) {
      StrOffset   = LastSpaceOffset;
      *GlyphWidth = LastGlyphWidth;
    } else {
      //
      // Roll back to last char in the line width.
      //
      StrOffset--;
    }
  }

  //
  // The CHAR_NULL has process last time, this time just return 0 to stand for the end.
  //
  if (StrOffset == 0 && (InputString[*Index + StrOffset] == CHAR_NULL)) {
    return 0;
  }

  //
  // Need extra glyph info and '\0' info, so +2.
  //
  *OutputString = AllocateZeroPool (((UINTN) (StrOffset + 2) * sizeof(CHAR16)));
  if (*OutputString == NULL) {
    return 0;
  }

  //
  // Save the glyph info at the begin of the string, will used by Print function.
  //
  if (OriginalGlyphWidth == 1) {
    *(*OutputString) = NARROW_CHAR;
  } else  {
    *(*OutputString) = WIDE_CHAR;
  }

  CopyMem ((*OutputString) + 1, &InputString[*Index], StrOffset * sizeof(CHAR16));

  if (InputString[*Index + StrOffset] == CHAR_SPACE) {
    //
    // Skip the space info at the begin of next line.
    //  
    *Index = (UINT16) (*Index + StrOffset + 1);
  } else if (InputString[*Index + StrOffset] == CHAR_LINEFEED) {
    //
    // Skip the /n or /n/r info.
    //
    if (InputString[*Index + StrOffset + 1] == CHAR_CARRIAGE_RETURN) {
      *Index = (UINT16) (*Index + StrOffset + 2);
    } else {
      *Index = (UINT16) (*Index + StrOffset + 1);
    }
  } else if (InputString[*Index + StrOffset] == CHAR_CARRIAGE_RETURN) {
    //
    // Skip the /r or /r/n info.
    //  
    if (InputString[*Index + StrOffset + 1] == CHAR_LINEFEED) {
      *Index = (UINT16) (*Index + StrOffset + 2);
    } else {
      *Index = (UINT16) (*Index + StrOffset + 1);
    }
  } else {
    *Index = (UINT16) (*Index + StrOffset);
  }

  //
  // Include extra glyph info and '\0' info, so +2.
  //
  return StrOffset + 2;
}

/**
  Add one menu option by specified description and context.

  @param  Statement              Statement of this Menu Option.
  @param  MenuItemCount          The index for this Option in the Menu.
  @param  NestIn                 Whether this statement is nest in another statement.

**/
VOID
UiAddMenuOption (
  IN FORM_DISPLAY_ENGINE_STATEMENT   *Statement,
  IN UINT16                          *MenuItemCount,
  IN BOOLEAN                         NestIn
  )
{
  UI_MENU_OPTION   *MenuOption;
  UINTN            Index;
  UINTN            Count;
  CHAR16           *String;
  UINT16           NumberOfLines;
  UINT16           GlyphWidth;
  UINT16           Width;
  UINTN            ArrayEntry;
  CHAR16           *OutputString;
  EFI_STRING_ID    PromptId;

  NumberOfLines = 1;
  ArrayEntry    = 0;
  GlyphWidth    = 1;
  Count         = 1;
  MenuOption    = NULL;

  PromptId = GetPrompt (Statement->OpCode);
  ASSERT (PromptId != 0);

  String = GetToken (PromptId, gFormData->HiiHandle);
  ASSERT (String != NULL);

  Width  = GetWidth (Statement);
  for (; GetLineByWidth (String, Width, &GlyphWidth,&ArrayEntry, &OutputString) != 0x0000;) {
    //
    // If there is more string to process print on the next row and increment the Skip value
    //
    if (StrLen (&String[ArrayEntry]) != 0) {
      NumberOfLines++;
    }
    FreePool (OutputString);
  }

  if (Statement->OpCode->OpCode == EFI_IFR_DATE_OP || Statement->OpCode->OpCode == EFI_IFR_TIME_OP) {
    //
    // Add three MenuOptions for Date/Time
    // Data format :      [01/02/2004]      [11:22:33]
    // Line number :        0  0    1         0  0  1
    //
    NumberOfLines = 0;
    Count = 3;
  }

  for (Index = 0; Index < Count; Index++) {
    MenuOption = AllocateZeroPool (sizeof (UI_MENU_OPTION));
    ASSERT (MenuOption);

    MenuOption->Signature   = UI_MENU_OPTION_SIGNATURE;
    MenuOption->Description = String;
    MenuOption->Handle      = gFormData->HiiHandle;
    MenuOption->ThisTag     = Statement;
    MenuOption->NestInStatement = NestIn;
    MenuOption->EntryNumber = *MenuItemCount;

    if (Index == 2) {
      //
      // Override LineNumber for the MenuOption in Date/Time sequence
      //
      MenuOption->Skip = 1;
    } else {
      MenuOption->Skip = NumberOfLines;
    }
    MenuOption->Sequence = Index;

    if ((Statement->Attribute & HII_DISPLAY_GRAYOUT) != 0) {
      MenuOption->GrayOut = TRUE;
    } else {
      MenuOption->GrayOut = FALSE;
    }

    if ((Statement->Attribute & HII_DISPLAY_LOCK) != 0 || (gFormData->Attribute & HII_DISPLAY_LOCK) != 0) {
      MenuOption->GrayOut = TRUE;
    }

    //
    // If the form or the question has the lock attribute, deal same as grayout.
    //
    if ((gFormData->Attribute & HII_DISPLAY_LOCK) != 0 || (Statement->Attribute & HII_DISPLAY_LOCK) != 0) {
      MenuOption->GrayOut = TRUE;
    }

    switch (Statement->OpCode->OpCode) {
    case EFI_IFR_ORDERED_LIST_OP:
    case EFI_IFR_ONE_OF_OP:
    case EFI_IFR_NUMERIC_OP:
    case EFI_IFR_TIME_OP:
    case EFI_IFR_DATE_OP:
    case EFI_IFR_CHECKBOX_OP:
    case EFI_IFR_PASSWORD_OP:
    case EFI_IFR_STRING_OP:
      //
      // User could change the value of these items
      //
      MenuOption->IsQuestion = TRUE;
      break;
    case EFI_IFR_TEXT_OP:
      if (FeaturePcdGet (PcdBrowserGrayOutTextStatement)) {
        //
        // Initializing GrayOut option as TRUE for Text setup options 
        // so that those options will be Gray in colour and un selectable.
        //
        MenuOption->GrayOut = TRUE;
      }
      break;
    default:
      MenuOption->IsQuestion = FALSE;
      break;
    }

    if ((Statement->Attribute & HII_DISPLAY_READONLY) != 0) {
      MenuOption->ReadOnly = TRUE;
      if (FeaturePcdGet (PcdBrowerGrayOutReadOnlyMenu)) {
        MenuOption->GrayOut = TRUE;
      }
    }

    InsertTailList (&gMenuOption, &MenuOption->Link);
  }

  (*MenuItemCount)++;
}

/**
  Create the menu list base on the form data info.

**/
VOID
ConvertStatementToMenu (
  VOID
  )
{
  UINT16                        MenuItemCount;
  LIST_ENTRY                    *Link;
  LIST_ENTRY                    *NestLink;
  FORM_DISPLAY_ENGINE_STATEMENT *Statement;
  FORM_DISPLAY_ENGINE_STATEMENT *NestStatement;

  MenuItemCount = 0;
  InitializeListHead (&gMenuOption);

  Link = GetFirstNode (&gFormData->StatementListHead);
  while (!IsNull (&gFormData->StatementListHead, Link)) {
    Statement = FORM_DISPLAY_ENGINE_STATEMENT_FROM_LINK (Link);
    Link = GetNextNode (&gFormData->StatementListHead, Link);

    //
    // Skip the opcode not recognized by Display core.
    //
    if (Statement->OpCode->OpCode == EFI_IFR_GUID_OP) {
      continue;
    }

    UiAddMenuOption (Statement, &MenuItemCount, FALSE);

    //
    // Check the statement nest in this host statement.
    //
    NestLink = GetFirstNode (&Statement->NestStatementList);
    while (!IsNull (&Statement->NestStatementList, NestLink)) {
      NestStatement = FORM_DISPLAY_ENGINE_STATEMENT_FROM_LINK (NestLink);
      NestLink = GetNextNode (&Statement->NestStatementList, NestLink);

      UiAddMenuOption (NestStatement, &MenuItemCount, TRUE);
    }
  }
}

/**
  Count the storage space of a Unicode string.

  This function handles the Unicode string with NARROW_CHAR
  and WIDE_CHAR control characters. NARROW_HCAR and WIDE_CHAR
  does not count in the resultant output. If a WIDE_CHAR is
  hit, then 2 Unicode character will consume an output storage
  space with size of CHAR16 till a NARROW_CHAR is hit.

  If String is NULL, then ASSERT ().

  @param String          The input string to be counted.

  @return Storage space for the input string.

**/
UINTN
GetStringWidth (
  IN CHAR16               *String
  )
{
  UINTN Index;
  UINTN Count;
  UINTN IncrementValue;

  ASSERT (String != NULL);
  if (String == NULL) {
    return 0;
  }

  Index           = 0;
  Count           = 0;
  IncrementValue  = 1;

  do {
    //
    // Advance to the null-terminator or to the first width directive
    //
    for (;
         (String[Index] != NARROW_CHAR) && (String[Index] != WIDE_CHAR) && (String[Index] != 0);
         Index++, Count = Count + IncrementValue
        )
      ;

    //
    // We hit the null-terminator, we now have a count
    //
    if (String[Index] == 0) {
      break;
    }
    //
    // We encountered a narrow directive - strip it from the size calculation since it doesn't get printed
    // and also set the flag that determines what we increment by.(if narrow, increment by 1, if wide increment by 2)
    //
    if (String[Index] == NARROW_CHAR) {
      //
      // Skip to the next character
      //
      Index++;
      IncrementValue = 1;
    } else {
      //
      // Skip to the next character
      //
      Index++;
      IncrementValue = 2;
    }
  } while (String[Index] != 0);

  //
  // Increment by one to include the null-terminator in the size
  //
  Count++;

  return Count * sizeof (CHAR16);
}

/**
  Base on the input option string to update the skip value for a menu option.

  @param  MenuOption             The MenuOption to be checked.
  @param  OptionString           The input option string.

**/
VOID
UpdateSkipInfoForMenu (
  IN UI_MENU_OPTION               *MenuOption,
  IN CHAR16                       *OptionString
  )
{
  UINTN   Index;
  UINT16  Width;
  UINTN   Row;
  CHAR16  *OutputString;
  UINT16  GlyphWidth;

  Width         = (UINT16) gOptionBlockWidth;
  GlyphWidth    = 1;
  Row           = 1;

  for (Index = 0; GetLineByWidth (OptionString, Width, &GlyphWidth, &Index, &OutputString) != 0x0000;) {
    if (StrLen (&OptionString[Index]) != 0) {
      Row++;
    }

    FreePool (OutputString);
  }

  if ((Row > MenuOption->Skip) && 
      (MenuOption->ThisTag->OpCode->OpCode != EFI_IFR_DATE_OP) && 
      (MenuOption->ThisTag->OpCode->OpCode != EFI_IFR_TIME_OP)) {
    MenuOption->Skip = Row;
  }
}

/**
  Update display lines for a Menu Option.

  @param  MenuOption             The MenuOption to be checked.

**/
VOID
UpdateOptionSkipLines (
  IN UI_MENU_OPTION               *MenuOption
  )
{
  CHAR16  *OptionString;

  OptionString  = NULL;

  ProcessOptions (MenuOption, FALSE, &OptionString, TRUE);
  if (OptionString != NULL) {
    UpdateSkipInfoForMenu (MenuOption, OptionString);

    FreePool (OptionString);
  }

  if ((MenuOption->ThisTag->OpCode->OpCode  == EFI_IFR_TEXT_OP) && (((EFI_IFR_TEXT*)MenuOption->ThisTag->OpCode)->TextTwo != 0)) {
    OptionString   = GetToken (((EFI_IFR_TEXT*)MenuOption->ThisTag->OpCode)->TextTwo, gFormData->HiiHandle);

    if (OptionString != NULL) {
      UpdateSkipInfoForMenu (MenuOption, OptionString);

      FreePool (OptionString);
    }
  }
}

/**
  Check whether this Menu Option could be highlighted.

  This is an internal function.

  @param  MenuOption             The MenuOption to be checked.

  @retval TRUE                   This Menu Option is selectable.
  @retval FALSE                  This Menu Option could not be selected.

**/
BOOLEAN
IsSelectable (
  UI_MENU_OPTION   *MenuOption
  )
{
  if ((MenuOption->ThisTag->OpCode->OpCode == EFI_IFR_SUBTITLE_OP) ||
      MenuOption->GrayOut || MenuOption->ReadOnly) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Move to next selectable statement.

  This is an internal function.

  @param  GoUp                   The navigation direction. TRUE: up, FALSE: down.
  @param  CurrentPosition        Current position.
  @param  GapToTop               Gap position to top or bottom.

  @return The row distance from current MenuOption to next selectable MenuOption.

  @retval -1       Reach the begin of the menu, still can't find the selectable menu.
  @retval Value    Find the selectable menu, maybe the truly selectable, maybe the l
                   last menu showing at current form.

**/
INTN
MoveToNextStatement (
  IN     BOOLEAN                   GoUp,
  IN OUT LIST_ENTRY                **CurrentPosition,
  IN     UINTN                     GapToTop
  )
{
  INTN             Distance;
  LIST_ENTRY       *Pos;
  UI_MENU_OPTION   *NextMenuOption;
  UI_MENU_OPTION   *PreMenuOption;

  Distance      = 0;
  Pos           = *CurrentPosition;
  PreMenuOption = MENU_OPTION_FROM_LINK (Pos);

  while (TRUE) {
    NextMenuOption = MENU_OPTION_FROM_LINK (Pos);
    //
    // NextMenuOption->Row == 0 means this menu has not calculate
    // the NextMenuOption->Skip value yet, just calculate here.
    //
    if (NextMenuOption->Row == 0) {
      UpdateOptionSkipLines (NextMenuOption);
    }
    
    if (GoUp && (PreMenuOption != NextMenuOption)) {
      //
      // In this case, still can't find the selectable menu,
      // return the last one in the showing form.
      //
      if ((UINTN) Distance + NextMenuOption->Skip > GapToTop) {
        NextMenuOption = PreMenuOption;
        break;
      }

      //
      // Current Position doesn't need to be caculated when go up.
      // Caculate distanct at first when go up
      //      
      Distance += NextMenuOption->Skip;
    }

    if (IsSelectable (NextMenuOption)) {
      break;
    }

    //
    // Arrive at begin of the menu list.
    //
    if ((GoUp ? Pos->BackLink : Pos->ForwardLink) == &gMenuOption) {
      Distance = -1;
      break;
    }

    if (!GoUp) {
      //
      // In this case, still can't find the selectable menu,
      // return the last one in the showing form.
      //
      if ((UINTN) Distance + NextMenuOption->Skip > GapToTop) {
        NextMenuOption = PreMenuOption;
        break;
      }

      Distance += NextMenuOption->Skip;
    }

    PreMenuOption = NextMenuOption;
    Pos = (GoUp ? Pos->BackLink : Pos->ForwardLink);
  }

  *CurrentPosition = &NextMenuOption->Link;
  return Distance;
}


/**
  Process option string for date/time opcode.

  @param  MenuOption              Menu option point to date/time.
  @param  OptionString            Option string input for process.
  @param  AddOptCol               Whether need to update MenuOption->OptCol. 

**/
VOID 
ProcessStringForDateTime (
  UI_MENU_OPTION                  *MenuOption,
  CHAR16                          *OptionString,
  BOOLEAN                         AddOptCol
  )
{
  UINTN Index;
  UINTN Count;
  FORM_DISPLAY_ENGINE_STATEMENT          *Statement;
  EFI_IFR_DATE                           *Date;
  EFI_IFR_TIME                           *Time;

  ASSERT (MenuOption != NULL && OptionString != NULL);
  
  Statement = MenuOption->ThisTag;
  Date      = NULL;
  Time      = NULL;
  if (Statement->OpCode->OpCode == EFI_IFR_DATE_OP) {
    Date = (EFI_IFR_DATE *) Statement->OpCode;
  } else if (Statement->OpCode->OpCode == EFI_IFR_TIME_OP) {
    Time = (EFI_IFR_TIME *) Statement->OpCode;
  }
  
  //
  // If leading spaces on OptionString - remove the spaces
  //
  for (Index = 0; OptionString[Index] == L' '; Index++) {
    //
    // Base on the blockspace to get the option column info.
    //
    if (AddOptCol) {
      MenuOption->OptCol++;
    }
  }
  
  for (Count = 0; OptionString[Index] != CHAR_NULL; Index++) {
    OptionString[Count] = OptionString[Index];
    Count++;
  }
  OptionString[Count] = CHAR_NULL;
  
  //
  // Enable to suppress field in the opcode base on the flag.
  //
  if (Statement->OpCode->OpCode == EFI_IFR_DATE_OP) {
    //
    // OptionString format is: <**:  **: ****>
    //                        |month|day|year|
    //                          4     3    5
    //
    if ((Date->Flags & EFI_QF_DATE_MONTH_SUPPRESS) && (MenuOption->Sequence == 0)) {
      //
      // At this point, only "<**:" in the optionstring. 
      // Clean the day's ** field, after clean, the format is "<  :"
      //
      SetUnicodeMem (&OptionString[1], 2, L' ');
    } else if ((Date->Flags & EFI_QF_DATE_DAY_SUPPRESS) && (MenuOption->Sequence == 1)) {
      //
      // At this point, only "**:" in the optionstring. 
      // Clean the month's "**" field, after clean, the format is "  :"
      //                
      SetUnicodeMem (&OptionString[0], 2, L' ');
    } else if ((Date->Flags & EFI_QF_DATE_YEAR_SUPPRESS) && (MenuOption->Sequence == 2)) {
      //
      // At this point, only "****>" in the optionstring. 
      // Clean the year's "****" field, after clean, the format is "  >"
      //                
      SetUnicodeMem (&OptionString[0], 4, L' ');
    }
  } else if (Statement->OpCode->OpCode == EFI_IFR_TIME_OP) {
    //
    // OptionString format is: <**:  **:    **>
    //                        |hour|minute|second|
    //                          4     3      3
    //
    if ((Time->Flags & QF_TIME_HOUR_SUPPRESS) && (MenuOption->Sequence == 0)) {
      //
      // At this point, only "<**:" in the optionstring. 
      // Clean the hour's ** field, after clean, the format is "<  :"
      //
      SetUnicodeMem (&OptionString[1], 2, L' ');
    } else if ((Time->Flags & QF_TIME_MINUTE_SUPPRESS) && (MenuOption->Sequence == 1)) {
      //
      // At this point, only "**:" in the optionstring. 
      // Clean the minute's "**" field, after clean, the format is "  :"
      //                
      SetUnicodeMem (&OptionString[0], 2, L' ');
    } else if ((Time->Flags & QF_TIME_SECOND_SUPPRESS) && (MenuOption->Sequence == 2)) {
      //
      // At this point, only "**>" in the optionstring. 
      // Clean the second's "**" field, after clean, the format is "  >"
      //                
      SetUnicodeMem (&OptionString[0], 2, L' ');
    }
  }
}


/**
  Adjust Data and Time position accordingly.
  Data format :      [01/02/2004]      [11:22:33]
  Line number :        0  0    1         0  0  1

  This is an internal function.

  @param  DirectionUp            the up or down direction. False is down. True is
                                 up.
  @param  CurrentPosition        Current position. On return: Point to the last
                                 Option (Year or Second) if up; Point to the first
                                 Option (Month or Hour) if down.

  @return Return line number to pad. It is possible that we stand on a zero-advance
  @return data or time opcode, so pad one line when we judge if we are going to scroll outside.

**/
UINTN
AdjustDateAndTimePosition (
  IN     BOOLEAN                     DirectionUp,
  IN OUT LIST_ENTRY                  **CurrentPosition
  )
{
  UINTN           Count;
  LIST_ENTRY      *NewPosition;
  UI_MENU_OPTION  *MenuOption;
  UINTN           PadLineNumber;

  PadLineNumber = 0;
  NewPosition   = *CurrentPosition;
  MenuOption    = MENU_OPTION_FROM_LINK (NewPosition);

  if ((MenuOption->ThisTag->OpCode->OpCode == EFI_IFR_DATE_OP) ||
      (MenuOption->ThisTag->OpCode->OpCode == EFI_IFR_TIME_OP)) {
    //
    // Calculate the distance from current position to the last Date/Time MenuOption
    //
    Count = 0;
    while (MenuOption->Skip == 0) {
      Count++;
      NewPosition   = NewPosition->ForwardLink;
      MenuOption    = MENU_OPTION_FROM_LINK (NewPosition);
      PadLineNumber = 1;
    }

    NewPosition = *CurrentPosition;
    if (DirectionUp) {
      //
      // Since the behavior of hitting the up arrow on a Date/Time MenuOption is intended
      // to be one that back to the previous set of MenuOptions, we need to advance to the first
      // Date/Time MenuOption and leave the remaining logic in CfUiUp intact so the appropriate
      // checking can be done.
      //
      while (Count++ < 2) {
        NewPosition = NewPosition->BackLink;
      }
    } else {
      //
      // Since the behavior of hitting the down arrow on a Date/Time MenuOption is intended
      // to be one that progresses to the next set of MenuOptions, we need to advance to the last
      // Date/Time MenuOption and leave the remaining logic in CfUiDown intact so the appropriate
      // checking can be done.
      //
      while (Count-- > 0) {
        NewPosition = NewPosition->ForwardLink;
      }
    }

    *CurrentPosition = NewPosition;
  }

  return PadLineNumber;
}

/**
  Get step info from numeric opcode.
  
  @param[in] OpCode     The input numeric op code.

  @return step info for this opcode.
**/
UINT64
GetFieldFromNum (
  IN  EFI_IFR_OP_HEADER     *OpCode
  )
{
  EFI_IFR_NUMERIC       *NumericOp;
  UINT64                Step;

  NumericOp = (EFI_IFR_NUMERIC *) OpCode;
  
  switch (NumericOp->Flags & EFI_IFR_NUMERIC_SIZE) {
  case EFI_IFR_NUMERIC_SIZE_1:
    Step    = NumericOp->data.u8.Step;
    break;
  
  case EFI_IFR_NUMERIC_SIZE_2:
    Step    = NumericOp->data.u16.Step;
    break;
  
  case EFI_IFR_NUMERIC_SIZE_4:
    Step    = NumericOp->data.u32.Step;
    break;
  
  case EFI_IFR_NUMERIC_SIZE_8:
    Step    = NumericOp->data.u64.Step;
    break;
  
  default:
    Step = 0;
    break;
  }

  return Step;
}

/**
  Find the registered HotKey based on KeyData.
  
  @param[in] KeyData     A pointer to a buffer that describes the keystroke
                         information for the hot key.

  @return The registered HotKey context. If no found, NULL will return.
**/
BROWSER_HOT_KEY *
GetHotKeyFromRegisterList (
  IN EFI_INPUT_KEY *KeyData
  )
{
  LIST_ENTRY       *Link;
  BROWSER_HOT_KEY  *HotKey;

  Link = GetFirstNode (&gFormData->HotKeyListHead);
  while (!IsNull (&gFormData->HotKeyListHead, Link)) {
    HotKey = BROWSER_HOT_KEY_FROM_LINK (Link);
    
    if (HotKey->KeyData->ScanCode == KeyData->ScanCode) {
      return HotKey;
    }

    Link = GetNextNode (&gFormData->HotKeyListHead, Link);
  }
  
  return NULL;
}


/**
  Determine if the menu is the last menu that can be selected.

  This is an internal function.

  @param  Direction              The scroll direction. False is down. True is up.
  @param  CurrentPos             The current focus.

  @return FALSE -- the menu isn't the last menu that can be selected.
  @return TRUE  -- the menu is the last menu that can be selected.

**/
BOOLEAN
ValueIsScroll (
  IN  BOOLEAN                     Direction,
  IN  LIST_ENTRY                  *CurrentPos
  )
{
  LIST_ENTRY      *Temp;

  Temp = Direction ? CurrentPos->BackLink : CurrentPos->ForwardLink;

  if (Temp == &gMenuOption) {
    return TRUE;
  }

  return FALSE;
}

/**
  Wait for a given event to fire, or for an optional timeout to expire.

  @param  Event                  The event to wait for

  @retval UI_EVENT_TYPE          The type of the event which is trigged.

**/
UI_EVENT_TYPE
UiWaitForEvent (
  IN EFI_EVENT                Event
  )
{
  EFI_STATUS  Status;
  UINTN       Index;
  UINTN       EventNum;
  UINT64      Timeout;
  EFI_EVENT   TimerEvent;
  EFI_EVENT   WaitList[3];
  UI_EVENT_TYPE  EventType;

  TimerEvent = NULL;
  Timeout    = FormExitTimeout(gFormData);

  if (Timeout != 0) {
    Status = gBS->CreateEvent (EVT_TIMER, 0, NULL, NULL, &TimerEvent);

    //
    // Set the timer event
    //
    gBS->SetTimer (
          TimerEvent,
          TimerRelative,
          Timeout
          );
  }
  
  WaitList[0] = Event;
  EventNum    = 1;
  if (gFormData->FormRefreshEvent != NULL) {
    WaitList[EventNum] = gFormData->FormRefreshEvent;
    EventNum ++;
  } 

  if (Timeout != 0) {
    WaitList[EventNum] = TimerEvent;
    EventNum ++;
  }

  Status = gBS->WaitForEvent (EventNum, WaitList, &Index);
  ASSERT_EFI_ERROR (Status);

  switch (Index) {
  case 0:
   EventType = UIEventKey;
   break;

  case 1:
    if (gFormData->FormRefreshEvent != NULL) {
      EventType = UIEventDriver;
    } else {
      ASSERT (Timeout != 0 && EventNum == 2);
      EventType = UIEventTimeOut;
    }
    break;

  default:
    ASSERT (Index == 2 && EventNum == 3);
    EventType = UIEventTimeOut;
    break;
  }

  if (Timeout != 0) {
    gBS->CloseEvent (TimerEvent);
  }
  
  return EventType;
}

/**
  Get question id info from the input opcode header.

  @param  OpCode                 The input opcode header pointer.

  @retval                        The question id for this opcode.

**/
EFI_QUESTION_ID
GetQuestionIdInfo (
  IN   EFI_IFR_OP_HEADER     *OpCode
  )
{
  EFI_IFR_QUESTION_HEADER   *QuestionHeader;

  if (OpCode->Length < sizeof (EFI_IFR_OP_HEADER) + sizeof (EFI_IFR_QUESTION_HEADER)) {
    return 0;
  }

  QuestionHeader = (EFI_IFR_QUESTION_HEADER *)((UINT8 *) OpCode + sizeof(EFI_IFR_OP_HEADER));

  return QuestionHeader->QuestionId;
}

/**
  Find the first menu which will be show at the top.

  @param  FormData               The data info for this form.
  @param  TopOfScreen            The link_entry pointer to top menu.
  @param  HighlightMenu          The menu which will be highlight.
  @param  SkipValue              The skip value for the top menu.

**/
VOID
FindTopMenu (
  IN  FORM_DISPLAY_ENGINE_FORM  *FormData,
  OUT LIST_ENTRY                **TopOfScreen,
  OUT LIST_ENTRY                **HighlightMenu,
  OUT INTN                      *SkipValue
  )
{
  LIST_ENTRY                      *Link;
  LIST_ENTRY                      *NewPos;
  UINTN                           TopRow;
  UINTN                           BottomRow;
  UINTN                           Index;
  UI_MENU_OPTION                  *SavedMenuOption;
  UINTN                           EndRow;

  TopRow    = gStatementDimensions.TopRow    + SCROLL_ARROW_HEIGHT;
  BottomRow = gStatementDimensions.BottomRow - SCROLL_ARROW_HEIGHT;

  //
  // If not has input highlight statement, just return the first one in this form.
  //
  if (FormData->HighLightedStatement == NULL) {
    *TopOfScreen   = gMenuOption.ForwardLink;
    *HighlightMenu = gMenuOption.ForwardLink;
    if (!IsListEmpty (&gMenuOption)) {
      MoveToNextStatement (FALSE, HighlightMenu, BottomRow - TopRow);
    }
    *SkipValue     = 0;
    return;
  }

  //
  // Now base on the input highlight menu to find the top menu in this page.
  // Will base on the highlight menu show at the bottom to find the top menu.
  //
  NewPos = gMenuOption.ForwardLink;
  SavedMenuOption = MENU_OPTION_FROM_LINK (NewPos);

  while ((SavedMenuOption->ThisTag != FormData->HighLightedStatement) ||
         (SavedMenuOption->Sequence != gSequence)) {
    NewPos     = NewPos->ForwardLink;
    if (NewPos == &gMenuOption) {
      //
      // Not Found it, break
      //
      break;
    }
    SavedMenuOption = MENU_OPTION_FROM_LINK (NewPos);
  }
  ASSERT (SavedMenuOption->ThisTag == FormData->HighLightedStatement);

  *HighlightMenu = NewPos;

  AdjustDateAndTimePosition(FALSE, &NewPos);
  SavedMenuOption = MENU_OPTION_FROM_LINK (NewPos);
  UpdateOptionSkipLines (SavedMenuOption);

  //
  // If highlight opcode is date/time, keep the highlight row info not change.
  //
  if ((SavedMenuOption->ThisTag->OpCode->OpCode == EFI_IFR_DATE_OP || SavedMenuOption->ThisTag->OpCode->OpCode == EFI_IFR_TIME_OP) &&
      (gHighligthMenuInfo.QuestionId != 0) && 
      (gHighligthMenuInfo.QuestionId == GetQuestionIdInfo(SavedMenuOption->ThisTag->OpCode))) {
    //
    // Still show the highlight menu before exit from display engine.
    //
    EndRow = gHighligthMenuInfo.DisplayRow + SavedMenuOption->Skip;
  } else {
    EndRow = BottomRow;
  }

  //
  // Base on the selected menu will show at the bottome of next page, 
  // select the menu show at the top of the next page. 
  //
  Link    = NewPos;
  for (Index = TopRow + SavedMenuOption->Skip; Index <= EndRow; ) {
    Link = Link->BackLink;
    //
    // Already find the first menu in this form, means highlight menu 
    // will show in first page of this form.
    //
    if (Link == &gMenuOption) {
      *TopOfScreen   = gMenuOption.ForwardLink;
      *SkipValue     = 0;
      return;
    }
    SavedMenuOption = MENU_OPTION_FROM_LINK (Link);
    UpdateOptionSkipLines (SavedMenuOption);
    Index += SavedMenuOption->Skip;
  }

  //
  // Found the menu which will show at the top of the page.
  //
  if (Link == NewPos) {
    //
    // The menu can show more than one pages, just show the menu at the top of the page.
    //
    *SkipValue    = 0;
    *TopOfScreen  = Link;
  } else {
    //
    // Check whether need to skip some line for menu shows at the top of the page.
    //
    *SkipValue = Index - EndRow;
    if (*SkipValue > 0 && *SkipValue < (INTN) SavedMenuOption->Skip) {
      *TopOfScreen     = Link;
    } else {
      *SkipValue       = 0;
      *TopOfScreen     = Link->ForwardLink;
    }
  }
}

/**
  Display menu and wait for user to select one menu option, then return it.
  If AutoBoot is enabled, then if user doesn't select any option,
  after period of time, it will automatically return the first menu option.

  @param  FormData               The current form data info.

  @retval EFI_SUCESSS            Process the user selection success.
  @retval EFI_NOT_FOUND          Process option string for orderedlist/Oneof fail.

**/
EFI_STATUS
UiDisplayMenu (
  IN  FORM_DISPLAY_ENGINE_FORM  *FormData
  )
{
  INTN                            SkipValue;
  INTN                            Difference;
  UINTN                           DistanceValue;
  UINTN                           Row;
  UINTN                           Col;
  UINTN                           TempRightCol;
  UINTN                           Temp;
  UINTN                           Temp2;
  UINTN                           Temp3;
  UINTN                           TopRow;
  UINTN                           BottomRow;
  UINTN                           OriginalRow;
  UINTN                           Index;
  UINT16                          Width;
  CHAR16                          *StringPtr;
  CHAR16                          *OptionString;
  CHAR16                          *OutputString;
  CHAR16                          *HelpString;
  CHAR16                          *HelpHeaderString;
  CHAR16                          *HelpBottomString;
  BOOLEAN                         NewLine;
  BOOLEAN                         Repaint;
  BOOLEAN                         UpArrow;
  BOOLEAN                         DownArrow;
  EFI_STATUS                      Status;
  EFI_INPUT_KEY                   Key;
  LIST_ENTRY                      *Link;
  LIST_ENTRY                      *NewPos;
  LIST_ENTRY                      *TopOfScreen;
  LIST_ENTRY                      *SavedListEntry;
  UI_MENU_OPTION                  *MenuOption;
  UI_MENU_OPTION                  *NextMenuOption;
  UI_MENU_OPTION                  *SavedMenuOption;
  UI_MENU_OPTION                  *PreviousMenuOption;
  UI_CONTROL_FLAG                 ControlFlag;
  UI_SCREEN_OPERATION             ScreenOperation;
  UINT16                          DefaultId;
  FORM_DISPLAY_ENGINE_STATEMENT   *Statement;
  UINTN                           ModalSkipColumn;
  BROWSER_HOT_KEY                 *HotKey;
  UINTN                           HelpPageIndex;
  UINTN                           HelpPageCount;
  UINTN                           RowCount;
  UINTN                           HelpLine;
  UINTN                           HelpHeaderLine;
  UINTN                           HelpBottomLine;
  BOOLEAN                         MultiHelpPage;
  UINT16                          GlyphWidth;
  UINT16                          EachLineWidth;
  UINT16                          HeaderLineWidth;
  UINT16                          BottomLineWidth;
  EFI_STRING_ID                   HelpInfo;
  UI_EVENT_TYPE                   EventType;
  FORM_DISPLAY_ENGINE_STATEMENT   *InitialHighlight;

  EventType           = UIEventNone;
  Status              = EFI_SUCCESS;
  HelpString          = NULL;
  HelpHeaderString    = NULL;
  HelpBottomString    = NULL;
  OptionString        = NULL;
  ScreenOperation     = UiNoOperation;
  NewLine             = TRUE;
  DefaultId           = 0;
  HelpPageCount       = 0;
  HelpLine            = 0;
  RowCount            = 0;
  HelpBottomLine      = 0;
  HelpHeaderLine      = 0;
  HelpPageIndex       = 0;
  MultiHelpPage       = FALSE;
  EachLineWidth       = 0;
  HeaderLineWidth     = 0;
  BottomLineWidth     = 0;
  OutputString        = NULL;
  UpArrow             = FALSE;
  DownArrow           = FALSE;
  SkipValue           = 0;

  NextMenuOption      = NULL;
  PreviousMenuOption  = NULL;
  SavedMenuOption     = NULL;
  HotKey              = NULL;
  Repaint             = TRUE;
  MenuOption          = NULL;
  ModalSkipColumn     = (gStatementDimensions.RightColumn - gStatementDimensions.LeftColumn) / 6;
  InitialHighlight    = gFormData->HighLightedStatement;

  ZeroMem (&Key, sizeof (EFI_INPUT_KEY));

  gOptionBlockWidth = (CHAR16) ((gStatementDimensions.RightColumn - gStatementDimensions.LeftColumn) / 3);
  gPromptBlockWidth = (CHAR16) (gOptionBlockWidth + LEFT_SKIPPED_COLUMNS);
  gHelpBlockWidth   = (CHAR16) (gOptionBlockWidth - LEFT_SKIPPED_COLUMNS);

  TopRow    = gStatementDimensions.TopRow    + SCROLL_ARROW_HEIGHT;
  BottomRow = gStatementDimensions.BottomRow - SCROLL_ARROW_HEIGHT - 1;

  Row = TopRow;
  if ((FormData->Attribute & HII_DISPLAY_MODAL) != 0) {
    Col = gStatementDimensions.LeftColumn + LEFT_SKIPPED_COLUMNS + ModalSkipColumn;
  } else {
    Col = gStatementDimensions.LeftColumn + LEFT_SKIPPED_COLUMNS;
  }

  FindTopMenu(FormData, &TopOfScreen, &NewPos, &SkipValue);

  gST->ConOut->EnableCursor (gST->ConOut, FALSE);

  ControlFlag = CfInitialization;
  while (TRUE) {
    switch (ControlFlag) {
    case CfInitialization:
      if (IsListEmpty (&gMenuOption)) {
        
        if ((FormData->Attribute & HII_DISPLAY_MODAL) == 0) {
          //
          // Clear Statement range.
          //
          ClearLines (
            gStatementDimensions.LeftColumn,
            gStatementDimensions.RightColumn,
            TopRow - SCROLL_ARROW_HEIGHT,
            BottomRow + SCROLL_ARROW_HEIGHT,
            GetFieldTextColor ()
            );
            
          //
          // Clear Key Range
          //
          RefreshKeyHelp (gFormData, NULL, FALSE);
        }

        ControlFlag = CfReadKey;
      } else {
        ControlFlag = CfRepaint;
      }
      break;

    case CfRepaint:
      ControlFlag = CfRefreshHighLight;

      if (Repaint) {
        //
        // Display menu
        //
        DownArrow       = FALSE;
        UpArrow         = FALSE;
        Row             = TopRow;

        Temp            = (UINTN) SkipValue;
        Temp2           = (UINTN) SkipValue;
        Temp3           = (UINTN) SkipValue;

        //
        // 1. Clear the screen.
        //
        if ((FormData->Attribute & HII_DISPLAY_MODAL) != 0) {
          ClearLines (
            gStatementDimensions.LeftColumn + ModalSkipColumn,
            gStatementDimensions.LeftColumn + ModalSkipColumn + gPromptBlockWidth + gOptionBlockWidth,
            TopRow - SCROLL_ARROW_HEIGHT,
            BottomRow + SCROLL_ARROW_HEIGHT,
            GetFieldTextColor ()
            );
        } else {
          TempRightCol = gStatementDimensions.RightColumn;
          if (!mStatementLayoutIsChanged) {
            TempRightCol = gStatementDimensions.RightColumn - gHelpBlockWidth;
          }
          ClearLines (
            gStatementDimensions.LeftColumn,
            gStatementDimensions.RightColumn,
            TopRow - SCROLL_ARROW_HEIGHT,
            TopRow - 1,
            GetFieldTextColor ()
            );
          ClearLines (
            gStatementDimensions.LeftColumn,
            TempRightCol,
            TopRow,
            BottomRow,
            GetFieldTextColor ()
            );
          ClearLines (
            gStatementDimensions.LeftColumn,
            gStatementDimensions.RightColumn,
            BottomRow + 1,
            BottomRow + SCROLL_ARROW_HEIGHT,
            GetFieldTextColor ()
            );
        }

        //
        // 2.Paint the menu.
        //
        for (Link = TopOfScreen; Link != &gMenuOption; Link = Link->ForwardLink) {
          MenuOption          = MENU_OPTION_FROM_LINK (Link);
          MenuOption->Row     = Row;
          MenuOption->Col     = Col;
          if ((FormData->Attribute & HII_DISPLAY_MODAL) != 0) {
            MenuOption->OptCol  = gPromptBlockWidth + 1 + gStatementDimensions.LeftColumn + ModalSkipColumn;
          } else {
            MenuOption->OptCol  = gPromptBlockWidth + 1 + gStatementDimensions.LeftColumn;
          }

          Statement = MenuOption->ThisTag;
          if (MenuOption->NestInStatement) {
            MenuOption->Col += SUBTITLE_INDENT;
          }

          if (MenuOption->GrayOut) {
            gST->ConOut->SetAttribute (gST->ConOut, GetGrayedTextColor ());
          } else {
            if (Statement->OpCode->OpCode == EFI_IFR_SUBTITLE_OP) {
              gST->ConOut->SetAttribute (gST->ConOut, GetSubTitleTextColor ());
            }
          }

          Width       = GetWidth (Statement);
          OriginalRow = Row;
          GlyphWidth  = 1;

          if (Statement->OpCode->OpCode == EFI_IFR_REF_OP && MenuOption->Col >= 2) {
            //
            // Print Arrow for Goto button.
            //
            PrintCharAt (
              MenuOption->Col - 2,
              Row,
              GEOMETRICSHAPE_RIGHT_TRIANGLE
              );
          }

          //
          // 2.1. Paint the description.
          //
          for (Index = 0; GetLineByWidth (MenuOption->Description, Width, &GlyphWidth, &Index, &OutputString) != 0x0000;) {
            //
            // Temp means need to skip how many lines from the start.
            //
            if ((Temp == 0) && (Row <= BottomRow)) {
              PrintStringAt (MenuOption->Col, Row, OutputString);
            }
            //
            // If there is more string to process print on the next row and increment the Skip value
            //
            if (StrLen (&MenuOption->Description[Index]) != 0) {
              if (Temp == 0) {
                Row++;
              }
            }

            FreePool (OutputString);
            if (Temp != 0) {
              Temp--;
            }
          }

          Temp  = 0;
          Row   = OriginalRow;

          //
          // 2.2. Paint the option string.
          //
          Status = ProcessOptions (MenuOption, FALSE, &OptionString, FALSE);
          //
          // If Error occur, question value update in ProcessOptions.
          // Exit current FormDisplay with new question value.
          //
          if (EFI_ERROR (Status)) {
            return Status;
          }

          if (OptionString != NULL) {
            if (Statement->OpCode->OpCode == EFI_IFR_DATE_OP || Statement->OpCode->OpCode == EFI_IFR_TIME_OP) {
              ProcessStringForDateTime(MenuOption, OptionString, TRUE);
            }

            Width       = (UINT16) gOptionBlockWidth;
            OriginalRow = Row;
            GlyphWidth  = 1;

            for (Index = 0; GetLineByWidth (OptionString, Width, &GlyphWidth, &Index, &OutputString) != 0x0000;) {
              if ((Temp2 == 0) && (Row <= BottomRow)) {
                PrintStringAt (MenuOption->OptCol, Row, OutputString);
              }
              //
              // If there is more string to process print on the next row and increment the Skip value
              //
              if (StrLen (&OptionString[Index]) != 0) {
                if (Temp2 == 0) {
                  Row++;
                  //
                  // Since the Number of lines for this menu entry may or may not be reflected accurately
                  // since the prompt might be 1 lines and option might be many, and vice versa, we need to do
                  // some testing to ensure we are keeping this in-sync.
                  //
                  // If the difference in rows is greater than or equal to the skip value, increase the skip value
                  //
                  if ((Row - OriginalRow) >= MenuOption->Skip) {
                    MenuOption->Skip++;
                  }
                }
              }

              FreePool (OutputString);
              if (Temp2 != 0) {
                Temp2--;
              }
            }

            Row   = OriginalRow;

            FreePool (OptionString);
          }
          Temp2 = 0;

          //
          // If this is a text op with secondary text information
          //
          if ((Statement->OpCode->OpCode  == EFI_IFR_TEXT_OP) && (((EFI_IFR_TEXT*)Statement->OpCode)->TextTwo != 0)) {
            StringPtr   = GetToken (((EFI_IFR_TEXT*)Statement->OpCode)->TextTwo, gFormData->HiiHandle);

            Width       = (UINT16) gOptionBlockWidth;
            OriginalRow = Row;
            GlyphWidth = 1;

            for (Index = 0; GetLineByWidth (StringPtr, Width, &GlyphWidth, &Index, &OutputString) != 0x0000;) {
              if ((Temp3 == 0) && (Row <= BottomRow)) {
                PrintStringAt (MenuOption->OptCol, Row, OutputString);
              }
              //
              // If there is more string to process print on the next row and increment the Skip value
              //
              if (StrLen (&StringPtr[Index]) != 0) {
                if (Temp3 == 0) {
                  Row++;
                  //
                  // Since the Number of lines for this menu entry may or may not be reflected accurately
                  // since the prompt might be 1 lines and option might be many, and vice versa, we need to do
                  // some testing to ensure we are keeping this in-sync.
                  //
                  // If the difference in rows is greater than or equal to the skip value, increase the skip value
                  //
                  if ((Row - OriginalRow) >= MenuOption->Skip) {
                    MenuOption->Skip++;
                  }
                }
              }

              FreePool (OutputString);
              if (Temp3 != 0) {
                Temp3--;
              }
            }

            Row = OriginalRow;
            FreePool (StringPtr);
          }
          Temp3 = 0;

          gST->ConOut->SetAttribute (gST->ConOut, GetFieldTextColor ());

          //
          // 3. Update the row info which will be used by next menu.
          //
          if (Link == TopOfScreen) {
            Row += MenuOption->Skip - SkipValue;
          } else {
            Row += MenuOption->Skip;
          }

          if (Row > BottomRow) {
            if (!ValueIsScroll (FALSE, Link)) {
              DownArrow = TRUE;
            }

            Row = BottomRow + 1;
            break;
          }
        }

        if (!ValueIsScroll (TRUE, TopOfScreen)) {
          UpArrow = TRUE;
        }

        if (UpArrow) {
          gST->ConOut->SetAttribute (gST->ConOut, GetArrowColor ());
          PrintCharAt (
            gStatementDimensions.LeftColumn + gPromptBlockWidth + gOptionBlockWidth + 1,
            TopRow - SCROLL_ARROW_HEIGHT,
            ARROW_UP
            );
          gST->ConOut->SetAttribute (gST->ConOut, GetFieldTextColor ());
        }

        if (DownArrow) {
          gST->ConOut->SetAttribute (gST->ConOut, GetArrowColor ());
          PrintCharAt (
            gStatementDimensions.LeftColumn + gPromptBlockWidth + gOptionBlockWidth + 1,
            BottomRow + SCROLL_ARROW_HEIGHT,
            ARROW_DOWN
            );
          gST->ConOut->SetAttribute (gST->ConOut, GetFieldTextColor ());
        }

        MenuOption = NULL;
      }
      break;

    case CfRefreshHighLight:

      //
      // MenuOption: Last menu option that need to remove hilight
      //             MenuOption is set to NULL in Repaint
      // NewPos:     Current menu option that need to hilight
      //
      ControlFlag = CfUpdateHelpString;

      if (MenuOption != NULL && TopOfScreen == &MenuOption->Link) {
        Temp = SkipValue;
      } else {
        Temp = 0;
      }
      if (NewPos == TopOfScreen) {
        Temp2 = SkipValue;
      } else {
        Temp2 = 0;
      }

      if (NewPos != NULL && (MenuOption == NULL || NewPos != &MenuOption->Link)) {
        if (MenuOption != NULL) {
          //
          // Remove highlight on last Menu Option
          //
          gST->ConOut->SetCursorPosition (gST->ConOut, MenuOption->Col, MenuOption->Row);
          ProcessOptions (MenuOption, FALSE, &OptionString, TRUE);
          gST->ConOut->SetAttribute (gST->ConOut, GetFieldTextColor ());
          if (OptionString != NULL) {
            if ((MenuOption->ThisTag->OpCode->OpCode == EFI_IFR_DATE_OP) ||
                (MenuOption->ThisTag->OpCode->OpCode == EFI_IFR_TIME_OP)
               ) {
              ProcessStringForDateTime(MenuOption, OptionString, FALSE);
            }

            Width               = (UINT16) gOptionBlockWidth;
            OriginalRow         = MenuOption->Row;
            GlyphWidth          = 1;

            for (Index = 0; GetLineByWidth (OptionString, Width, &GlyphWidth, &Index, &OutputString) != 0x0000;) {
              if ((Temp == 0) && (MenuOption->Row >= TopRow) && (MenuOption->Row <= BottomRow)) {
                PrintStringAt (MenuOption->OptCol, MenuOption->Row, OutputString);
              }
              //
              // If there is more string to process print on the next row and increment the Skip value
              //
              if (StrLen (&OptionString[Index]) != 0) {
                if (Temp == 0) {
                  MenuOption->Row++;
                }
              }

              FreePool (OutputString);
              if (Temp != 0) {
                Temp--;
              }
            }

            MenuOption->Row = OriginalRow;

            FreePool (OptionString);
          } else {
            if (NewLine) {
              if (MenuOption->GrayOut) {
                gST->ConOut->SetAttribute (gST->ConOut, GetGrayedTextColor ());
              } else if (MenuOption->ThisTag->OpCode->OpCode == EFI_IFR_SUBTITLE_OP) {
                gST->ConOut->SetAttribute (gST->ConOut, GetSubTitleTextColor ());
              }

              OriginalRow = MenuOption->Row;
              Width       = GetWidth (MenuOption->ThisTag);
              GlyphWidth  = 1;

              for (Index = 0; GetLineByWidth (MenuOption->Description, Width, &GlyphWidth, &Index, &OutputString) != 0x0000;) {
                if ((Temp == 0) && (MenuOption->Row >= TopRow) && (MenuOption->Row <= BottomRow)) {
                  PrintStringAt (MenuOption->Col, MenuOption->Row, OutputString);
                }
                //
                // If there is more string to process print on the next row and increment the Skip value
                //
                if (StrLen (&MenuOption->Description[Index]) != 0) {
                  if (Temp == 0) {
                    MenuOption->Row++;
                  }
                }

                FreePool (OutputString);
                if (Temp != 0) {
                  Temp--;
                }
              }

              MenuOption->Row = OriginalRow;
              gST->ConOut->SetAttribute (gST->ConOut, GetFieldTextColor ());
            }
          }
        }

        //
        // This is the current selected statement
        //
        MenuOption = MENU_OPTION_FROM_LINK (NewPos);
        Statement = MenuOption->ThisTag;

        //
        // Get the highlight statement.
        //
        gUserInput->SelectedStatement = Statement;
        gSequence = (UINT16) MenuOption->Sequence;

        //
        // Record highlight row info for date/time opcode.
        //
        if (Statement->OpCode->OpCode == EFI_IFR_DATE_OP || Statement->OpCode->OpCode == EFI_IFR_TIME_OP) {
          gHighligthMenuInfo.QuestionId = GetQuestionIdInfo(Statement->OpCode);
          gHighligthMenuInfo.DisplayRow = (UINT16) MenuOption->Row;
        } else {
          gHighligthMenuInfo.QuestionId = 0;
          gHighligthMenuInfo.DisplayRow = 0;
        }

        if (!IsSelectable (MenuOption)) {
          RefreshKeyHelp(gFormData, Statement, FALSE);
          break;
        }

        //
        // Set reverse attribute
        //
        gST->ConOut->SetAttribute (gST->ConOut, GetHighlightTextColor ());
        gST->ConOut->SetCursorPosition (gST->ConOut, MenuOption->Col, MenuOption->Row);

        ProcessOptions (MenuOption, FALSE, &OptionString, TRUE);
        if (OptionString != NULL) {
          if (Statement->OpCode->OpCode == EFI_IFR_DATE_OP || Statement->OpCode->OpCode == EFI_IFR_TIME_OP) {
            ProcessStringForDateTime(MenuOption, OptionString, FALSE);
          }
          Width               = (UINT16) gOptionBlockWidth;

          OriginalRow         = MenuOption->Row;
          GlyphWidth          = 1;

          for (Index = 0; GetLineByWidth (OptionString, Width, &GlyphWidth, &Index, &OutputString) != 0x0000;) {
            if ((Temp2 == 0) && (MenuOption->Row >= TopRow) && (MenuOption->Row <= BottomRow) ) {
              PrintStringAt (MenuOption->OptCol, MenuOption->Row, OutputString);
            }
            //
            // If there is more string to process print on the next row and increment the Skip value
            //
            if (StrLen (&OptionString[Index]) != 0) {
              if (Temp2 == 0) {
              MenuOption->Row++;
              }
            }

            FreePool (OutputString);
            if (Temp2 != 0) {
              Temp2--;
            }
          }

          MenuOption->Row = OriginalRow;

          FreePool (OptionString);
        } else {
          if (NewLine) {
            OriginalRow = MenuOption->Row;

            Width       = GetWidth (Statement);
            GlyphWidth          = 1;

            for (Index = 0; GetLineByWidth (MenuOption->Description, Width, &GlyphWidth, &Index, &OutputString) != 0x0000;) {
              if ((Temp2 == 0) && (MenuOption->Row >= TopRow) && (MenuOption->Row <= BottomRow) ) {
                PrintStringAt (MenuOption->Col, MenuOption->Row, OutputString);
              }
              //
              // If there is more string to process print on the next row and increment the Skip value
              //
              if (StrLen (&MenuOption->Description[Index]) != 0) {
                if (Temp2 == 0) {
                  MenuOption->Row++;
                }
              }

              FreePool (OutputString);
              if (Temp2 != 0) {
                Temp2--;
              }
            }

            MenuOption->Row = OriginalRow;

          }
        }

        RefreshKeyHelp(gFormData, MenuOption->ThisTag, FALSE);

        //
        // Clear reverse attribute
        //
        gST->ConOut->SetAttribute (gST->ConOut, GetFieldTextColor ());
      }
      break;

    case CfUpdateHelpString:
      ControlFlag = CfPrepareToReadKey;
      if ((FormData->Attribute & HII_DISPLAY_MODAL) != 0) {
        break;
      }

      if (Repaint || NewLine) {
        //
        // Don't print anything if it is a NULL help token
        //
        ASSERT(MenuOption != NULL);
        HelpInfo = ((EFI_IFR_STATEMENT_HEADER *) ((CHAR8 *)MenuOption->ThisTag->OpCode + sizeof (EFI_IFR_OP_HEADER)))->Help;
        if (HelpInfo == 0 || !IsSelectable (MenuOption)) {
          StringPtr = GetToken (STRING_TOKEN (EMPTY_STRING), gHiiHandle);
        } else {
          StringPtr = GetToken (HelpInfo, gFormData->HiiHandle);
        }

        RowCount      = BottomRow - TopRow + 1;
        HelpPageIndex = 0;
        //
        // 1.Calculate how many line the help string need to print.
        //
        if (HelpString != NULL) {
          FreePool (HelpString);
          HelpString = NULL;
        }
        HelpLine = ProcessHelpString (StringPtr, &HelpString, &EachLineWidth, RowCount);
        FreePool (StringPtr);

        if (HelpLine > RowCount) {
          MultiHelpPage   = TRUE;
          StringPtr       = GetToken (STRING_TOKEN(ADJUST_HELP_PAGE_UP), gHiiHandle);
          if (HelpHeaderString != NULL) {
            FreePool (HelpHeaderString);
            HelpHeaderString = NULL;
          }
          HelpHeaderLine  = ProcessHelpString (StringPtr, &HelpHeaderString, &HeaderLineWidth, 0);
          FreePool (StringPtr);
          StringPtr       = GetToken (STRING_TOKEN(ADJUST_HELP_PAGE_DOWN), gHiiHandle);
          if (HelpBottomString != NULL) {
            FreePool (HelpBottomString);
            HelpBottomString = NULL;
          }
          HelpBottomLine  = ProcessHelpString (StringPtr, &HelpBottomString, &BottomLineWidth, 0);
          FreePool (StringPtr);
          //
          // Calculate the help page count.
          //
          if (HelpLine > 2 * RowCount - 2) {
            HelpPageCount = (HelpLine - RowCount + 1) / (RowCount - 2) + 1;
            if ((HelpLine - RowCount + 1) % (RowCount - 2) > 1) {
              HelpPageCount += 1;
            }
          } else {
            HelpPageCount = 2;
          }
        } else {
          MultiHelpPage = FALSE;
        }
      }

      //
      // Check whether need to show the 'More(U/u)' at the begin.
      // Base on current direct info, here shows aligned to the right side of the column.
      // If the direction is multi line and aligned to right side may have problem, so 
      // add ASSERT code here.
      //
      if (HelpPageIndex > 0) {
        gST->ConOut->SetAttribute (gST->ConOut, GetInfoTextColor ());
        for (Index = 0; Index < HelpHeaderLine; Index++) {
          ASSERT (HelpHeaderLine == 1);
          ASSERT (GetStringWidth (HelpHeaderString) / 2 < (UINTN) (gHelpBlockWidth - 1));
          PrintStringAtWithWidth (
            gStatementDimensions.RightColumn - gHelpBlockWidth,
            Index + TopRow,
            gEmptyString,
            gHelpBlockWidth
            );
          PrintStringAt (
            gStatementDimensions.RightColumn - GetStringWidth (HelpHeaderString) / 2 - 1,
            Index + TopRow,
            &HelpHeaderString[Index * HeaderLineWidth]
            );
        }
      }

      gST->ConOut->SetAttribute (gST->ConOut, GetHelpTextColor ());
      //
      // Print the help string info.
      //
      if (!MultiHelpPage) {
        for (Index = 0; Index < HelpLine; Index++) {
          PrintStringAtWithWidth (
            gStatementDimensions.RightColumn - gHelpBlockWidth,
            Index + TopRow,
            &HelpString[Index * EachLineWidth],
            gHelpBlockWidth
            );
        }
        for (; Index < RowCount; Index ++) {
          PrintStringAtWithWidth (
            gStatementDimensions.RightColumn - gHelpBlockWidth,
            Index + TopRow,
            gEmptyString,
            gHelpBlockWidth
            );
        }
        gST->ConOut->SetCursorPosition(gST->ConOut, gStatementDimensions.RightColumn-1, BottomRow);
      } else  {
        if (HelpPageIndex == 0) {
          for (Index = 0; Index < RowCount - HelpBottomLine; Index++) {
            PrintStringAtWithWidth (
              gStatementDimensions.RightColumn - gHelpBlockWidth,
              Index + TopRow,
              &HelpString[Index * EachLineWidth],
              gHelpBlockWidth
              );
          }
        } else {
          for (Index = 0; (Index < RowCount - HelpBottomLine - HelpHeaderLine) && 
              (Index + HelpPageIndex * (RowCount - 2) + 1 < HelpLine); Index++) {
            PrintStringAtWithWidth (
              gStatementDimensions.RightColumn - gHelpBlockWidth,
              Index + TopRow + HelpHeaderLine,
              &HelpString[(Index + HelpPageIndex * (RowCount - 2) + 1)* EachLineWidth],
              gHelpBlockWidth
              );
          }
          if (HelpPageIndex == HelpPageCount - 1) {
            for (; Index < RowCount - HelpHeaderLine; Index ++) {
              PrintStringAtWithWidth (
                gStatementDimensions.RightColumn - gHelpBlockWidth,
                Index + TopRow + HelpHeaderLine,
                gEmptyString,
                gHelpBlockWidth
                );
            }
            gST->ConOut->SetCursorPosition(gST->ConOut, gStatementDimensions.RightColumn-1, BottomRow);
          }
        } 
      }

      //
      // Check whether need to print the 'More(D/d)' at the bottom.
      // Base on current direct info, here shows aligned to the right side of the column.
      // If the direction is multi line and aligned to right side may have problem, so 
      // add ASSERT code here.
      //
      if (HelpPageIndex < HelpPageCount - 1 && MultiHelpPage) {
        gST->ConOut->SetAttribute (gST->ConOut, GetInfoTextColor ());
        for (Index = 0; Index < HelpBottomLine; Index++) {
          ASSERT (HelpBottomLine == 1);
          ASSERT (GetStringWidth (HelpBottomString) / 2 < (UINTN) (gHelpBlockWidth - 1)); 
          PrintStringAtWithWidth (
            gStatementDimensions.RightColumn - gHelpBlockWidth,
            BottomRow + Index - HelpBottomLine + 1,
            gEmptyString,
            gHelpBlockWidth
            );
          PrintStringAt (
            gStatementDimensions.RightColumn - GetStringWidth (HelpBottomString) / 2 - 1,
            BottomRow + Index - HelpBottomLine + 1,
            &HelpBottomString[Index * BottomLineWidth]
            );
        }
      }
      //
      // Reset this flag every time we finish using it.
      //
      Repaint = FALSE;
      NewLine = FALSE;
      break;

    case CfPrepareToReadKey:
      ControlFlag = CfReadKey;
      ScreenOperation = UiNoOperation;
      break;

    case CfReadKey:
      ControlFlag = CfScreenOperation;

      //
      // Wait for user's selection
      //
      while (TRUE) {
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
        if (!EFI_ERROR (Status)) {
          EventType = UIEventKey;
          break;
        }

        //
        // If we encounter error, continue to read another key in.
        //
        if (Status != EFI_NOT_READY) {
          continue;
        }
        
        EventType = UiWaitForEvent(gST->ConIn->WaitForKey);
        if (EventType == UIEventKey) {
          gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
        }
        break;
      }

      if (EventType == UIEventDriver) {
        gUserInput->Action = BROWSER_ACTION_NONE;
        ControlFlag = CfExit;
        break;
      }
      
      if (EventType == UIEventTimeOut) {
        gUserInput->Action = BROWSER_ACTION_FORM_EXIT;
        ControlFlag = CfExit;
        break;
      }

      switch (Key.UnicodeChar) {
      case CHAR_CARRIAGE_RETURN:
        if(MenuOption == NULL || MenuOption->GrayOut || MenuOption->ReadOnly) {
          ControlFlag = CfReadKey;
          break;
        }

        ScreenOperation = UiSelect;
        gDirection      = 0;
        break;

      //
      // We will push the adjustment of these numeric values directly to the input handler
      //  NOTE: we won't handle manual input numeric
      //
      case '+':
      case '-':
        //
        // If the screen has no menu items, and the user didn't select UiReset
        // ignore the selection and go back to reading keys.
        //
        if(IsListEmpty (&gMenuOption) || MenuOption->GrayOut || MenuOption->ReadOnly) {
          ControlFlag = CfReadKey;
          break;
        }

        ASSERT(MenuOption != NULL);
        Statement = MenuOption->ThisTag;
        if ((Statement->OpCode->OpCode == EFI_IFR_DATE_OP)
          || (Statement->OpCode->OpCode == EFI_IFR_TIME_OP)
          || ((Statement->OpCode->OpCode == EFI_IFR_NUMERIC_OP) && (GetFieldFromNum(Statement->OpCode) != 0))
        ){
          if (Key.UnicodeChar == '+') {
            gDirection = SCAN_RIGHT;
          } else {
            gDirection = SCAN_LEFT;
          }
          
          Status = ProcessOptions (MenuOption, TRUE, &OptionString, TRUE);
          if (OptionString != NULL) {
            FreePool (OptionString);
          }
          if (EFI_ERROR (Status)) {
            //
            // Repaint to clear possible error prompt pop-up
            //
            Repaint = TRUE;
            NewLine = TRUE;
          } else {
            ControlFlag = CfExit;
          }
        }
        break;

      case '^':
        ScreenOperation = UiUp;
        break;

      case 'V':
      case 'v':
        ScreenOperation = UiDown;
        break;

      case ' ':
        if(IsListEmpty (&gMenuOption)) {
          ControlFlag = CfReadKey;
          break;
        }
        
        ASSERT(MenuOption != NULL);
        if (MenuOption->ThisTag->OpCode->OpCode == EFI_IFR_CHECKBOX_OP && !MenuOption->GrayOut && !MenuOption->ReadOnly) {
          ScreenOperation = UiSelect;
        }
        break;

      case 'D':
      case 'd':
        if (!MultiHelpPage) {
          ControlFlag = CfReadKey;
          break;
        }
        ControlFlag    = CfUpdateHelpString;
        HelpPageIndex  = HelpPageIndex < HelpPageCount - 1 ? HelpPageIndex + 1 : HelpPageCount - 1;
        break;

      case 'U':
      case 'u':
        if (!MultiHelpPage) {
          ControlFlag = CfReadKey;
          break;
        }
        ControlFlag    = CfUpdateHelpString;
        HelpPageIndex  = HelpPageIndex > 0 ? HelpPageIndex - 1 : 0;
        break;

      case CHAR_NULL:
        for (Index = 0; Index < mScanCodeNumber; Index++) {
          if (Key.ScanCode == gScanCodeToOperation[Index].ScanCode) {
            ScreenOperation = gScanCodeToOperation[Index].ScreenOperation;
            break;
          }
        }
        
        if (((FormData->Attribute & HII_DISPLAY_MODAL) != 0) && (Key.ScanCode == SCAN_ESC || Index == mScanCodeNumber)) {
          //
          // ModalForm has no ESC key and Hot Key.
          //
          ControlFlag = CfReadKey;
        } else if (Index == mScanCodeNumber) {
          //
          // Check whether Key matches the registered hot key.
          //
          HotKey = NULL;
          HotKey = GetHotKeyFromRegisterList (&Key);
          if (HotKey != NULL) {
            ScreenOperation = UiHotKey;
          }
        }
        break;
      }
      break;

    case CfScreenOperation:
      if (ScreenOperation != UiReset) {
        //
        // If the screen has no menu items, and the user didn't select UiReset
        // ignore the selection and go back to reading keys.
        //
        if (IsListEmpty (&gMenuOption)) {
          ControlFlag = CfReadKey;
          break;
        }
      }

      for (Index = 0;
           Index < sizeof (gScreenOperationToControlFlag) / sizeof (gScreenOperationToControlFlag[0]);
           Index++
          ) {
        if (ScreenOperation == gScreenOperationToControlFlag[Index].ScreenOperation) {
          ControlFlag = gScreenOperationToControlFlag[Index].ControlFlag;
          break;
        }
      }
      break;

    case CfUiSelect:
      ControlFlag = CfRepaint;

      ASSERT(MenuOption != NULL);
      Statement = MenuOption->ThisTag;
      if (Statement->OpCode->OpCode == EFI_IFR_TEXT_OP) {
        break;
      }

      switch (Statement->OpCode->OpCode) {
      case EFI_IFR_REF_OP:
      case EFI_IFR_ACTION_OP:
      case EFI_IFR_RESET_BUTTON_OP:
        ControlFlag = CfExit;
        break;

      default:
        //
        // Editable Questions: oneof, ordered list, checkbox, numeric, string, password
        //
        RefreshKeyHelp (gFormData, Statement, TRUE);
        Status = ProcessOptions (MenuOption, TRUE, &OptionString, TRUE);
        
        if (OptionString != NULL) {
          FreePool (OptionString);
        }
        
        if (EFI_ERROR (Status)) {
          Repaint = TRUE;
          NewLine = TRUE;
          RefreshKeyHelp (gFormData, Statement, FALSE);
          break;
        } else {
          ControlFlag = CfExit;
          break;
        }
      }
      break;

    case CfUiReset:
      //
      // We come here when someone press ESC
      // If the policy is not exit front page when user press ESC, process here.
      //
      if (!FormExitPolicy()) {
        Repaint     = TRUE;
        NewLine     = TRUE;
        ControlFlag = CfRepaint;
        break;
      }

      //
      // When user press ESC, it will try to show another menu, should clean the gSequence info.
      //
      if (gSequence != 0) {
        gSequence = 0;
      }

      gUserInput->Action = BROWSER_ACTION_FORM_EXIT;
      ControlFlag = CfExit;
      break;

    case CfUiHotKey:
      ControlFlag = CfRepaint;
      
      gUserInput->Action = HotKey->Action;
      ControlFlag = CfExit;
      break;

    case CfUiLeft:
      ControlFlag = CfRepaint;
      ASSERT(MenuOption != NULL);
      if ((MenuOption->ThisTag->OpCode->OpCode == EFI_IFR_DATE_OP) || (MenuOption->ThisTag->OpCode->OpCode == EFI_IFR_TIME_OP)) {
        if (MenuOption->Sequence != 0) {
          //
          // In the middle or tail of the Date/Time op-code set, go left.
          //
          ASSERT(NewPos != NULL);
          NewPos = NewPos->BackLink;
        }
      }
      break;

    case CfUiRight:
      ControlFlag = CfRepaint;
      ASSERT(MenuOption != NULL);
      if ((MenuOption->ThisTag->OpCode->OpCode == EFI_IFR_DATE_OP) || (MenuOption->ThisTag->OpCode->OpCode == EFI_IFR_TIME_OP)) {
        if (MenuOption->Sequence != 2) {
          //
          // In the middle or tail of the Date/Time op-code set, go left.
          //
          ASSERT(NewPos != NULL);
          NewPos = NewPos->ForwardLink;
        }
      }
      break;

    case CfUiUp:
      ControlFlag = CfRepaint;

      SavedListEntry = NewPos;

      ASSERT(NewPos != NULL);
      //
      // Adjust Date/Time position before we advance forward.
      //
      AdjustDateAndTimePosition (TRUE, &NewPos);
      if (NewPos->BackLink != &gMenuOption) {
        MenuOption = MENU_OPTION_FROM_LINK (NewPos);
        ASSERT (MenuOption != NULL);
        NewLine    = TRUE;
        NewPos     = NewPos->BackLink;

        PreviousMenuOption = MENU_OPTION_FROM_LINK (NewPos);
        if (PreviousMenuOption->Row == 0) {
          UpdateOptionSkipLines (PreviousMenuOption);
        }
        DistanceValue = PreviousMenuOption->Skip;
        Difference    = 0;
        if (MenuOption->Row >= DistanceValue + TopRow) {
          Difference = MoveToNextStatement (TRUE, &NewPos, MenuOption->Row - TopRow - DistanceValue);
        }
        NextMenuOption = MENU_OPTION_FROM_LINK (NewPos);
       
        if (Difference < 0) {
          //
          // We hit the begining MenuOption that can be focused
          // so we simply scroll to the top.
          //
          if (TopOfScreen != gMenuOption.ForwardLink) {
            TopOfScreen = gMenuOption.ForwardLink;
            Repaint     = TRUE;
          } else {
            //
            // Scroll up to the last page when we have arrived at top page.
            //
            NewPos          = &gMenuOption;
            TopOfScreen     = &gMenuOption;
            MenuOption      = MENU_OPTION_FROM_LINK (SavedListEntry);
            ScreenOperation = UiPageUp;
            ControlFlag     = CfScreenOperation;
            break;
          }
        } else if (MenuOption->Row < TopRow + DistanceValue + Difference) {
          //
          // Previous focus MenuOption is above the TopOfScreen, so we need to scroll
          //
          TopOfScreen = NewPos;
          Repaint     = TRUE;
          SkipValue = 0;
        } else if (!IsSelectable (NextMenuOption)) {
          //
          // Continue to go up until scroll to next page or the selectable option is found.
          //
          ScreenOperation = UiUp;
          ControlFlag     = CfScreenOperation;
        }

        //
        // If we encounter a Date/Time op-code set, rewind to the first op-code of the set.
        //
        AdjustDateAndTimePosition (TRUE, &TopOfScreen);
        AdjustDateAndTimePosition (TRUE, &NewPos);
        MenuOption = MENU_OPTION_FROM_LINK (SavedListEntry);
        UpdateStatusBar (INPUT_ERROR, FALSE);
      } else {
        //
        // Scroll up to the last page.
        //
        NewPos          = &gMenuOption;
        TopOfScreen     = &gMenuOption;
        MenuOption      = MENU_OPTION_FROM_LINK (SavedListEntry);
        ScreenOperation = UiPageUp;
        ControlFlag     = CfScreenOperation;
      }
      break;

    case CfUiPageUp:
      //
      // SkipValue means lines is skipped when show the top menu option.
      //
      ControlFlag     = CfRepaint;

      ASSERT(NewPos != NULL);
      //
      // Already at the first menu option, Check the skip value.
      //
      if (NewPos->BackLink == &gMenuOption) {
        if (SkipValue == 0) {
          NewLine = FALSE;
          Repaint = FALSE;
        } else {
          NewLine = TRUE;
          Repaint = TRUE;
          SkipValue = 0;
        }
        break;
      }

      NewLine   = TRUE;
      Repaint   = TRUE;

      //
      // SkipValue > (BottomRow - TopRow + 1) means current menu has more than one
      // form of options to be show, so just update the SkipValue to show the next
      // parts of options.
      //
      if (SkipValue > (INTN) (BottomRow - TopRow + 1)) {
        SkipValue -= BottomRow - TopRow + 1;
        break;
      }

      Link      = TopOfScreen;
      //
      // First minus the menu of the top screen, it's value is SkipValue.
      //
      Index     = (BottomRow + 1) - SkipValue;
      while ((Index > TopRow) && (Link->BackLink != &gMenuOption)) {
        Link = Link->BackLink;
        PreviousMenuOption = MENU_OPTION_FROM_LINK (Link);
        if (PreviousMenuOption->Row == 0) {
          UpdateOptionSkipLines (PreviousMenuOption);
        }        
        if (Index < PreviousMenuOption->Skip) {
          break;
        }
        Index = Index - PreviousMenuOption->Skip;
      }
      
      if ((Link->BackLink == &gMenuOption) && (Index >= TopRow)) {
        SkipValue = 0;
        if (TopOfScreen == &gMenuOption) {
          TopOfScreen = gMenuOption.ForwardLink;
          NewPos      = gMenuOption.BackLink;
          MoveToNextStatement (TRUE, &NewPos, BottomRow - TopRow);
          Repaint = FALSE;
        } else if (TopOfScreen != Link) {
          TopOfScreen = Link;
          NewPos      = Link;
          MoveToNextStatement (FALSE, &NewPos, BottomRow - TopRow);
        } else {
          //
          // Finally we know that NewPos is the last MenuOption can be focused.
          //
          Repaint = FALSE;
          NewPos  = Link;
          MoveToNextStatement (FALSE, &NewPos, BottomRow - TopRow);
        }
      } else {
        if (Index > TopRow) {
          //
          // At here, only case "Index < PreviousMenuOption->Skip" can reach here.
          //
          SkipValue = PreviousMenuOption->Skip - (Index - TopRow);
        } else if (Index == TopRow) {
          SkipValue = 0;
        } else {
          SkipValue = TopRow - Index;
        }

        //
        // Move to the option in Next page.
        //
        if (TopOfScreen == &gMenuOption) {
          NewPos = gMenuOption.BackLink;
          MoveToNextStatement (TRUE, &NewPos, BottomRow - TopRow);
        } else {
          NewPos = Link;
          MoveToNextStatement (FALSE, &NewPos, BottomRow - TopRow);
        }

        //
        // There are more MenuOption needing scrolling up.
        //
        TopOfScreen = Link;
        MenuOption  = NULL;
      }

      //
      // If we encounter a Date/Time op-code set, rewind to the first op-code of the set.
      // Don't do this when we are already in the first page.
      //
      AdjustDateAndTimePosition (TRUE, &TopOfScreen);
      AdjustDateAndTimePosition (TRUE, &NewPos);
      break;

    case CfUiPageDown:
      //
      // SkipValue means lines is skipped when show the top menu option.
      //
      ControlFlag     = CfRepaint;

      ASSERT (NewPos != NULL);
      if (NewPos->ForwardLink == &gMenuOption) {
        NewLine = FALSE;
        Repaint = FALSE;
        break;
      }

      NewLine = TRUE;
      Repaint = TRUE;
      Link    = TopOfScreen;
      NextMenuOption = MENU_OPTION_FROM_LINK (Link);
      Index = TopRow + NextMenuOption->Skip - SkipValue;
      //
      // Count to the menu option which will show at the top of the next form.
      //
      while ((Index <= BottomRow + 1) && (Link->ForwardLink != &gMenuOption)) {
        Link           = Link->ForwardLink;
        NextMenuOption = MENU_OPTION_FROM_LINK (Link);
        Index = Index + NextMenuOption->Skip;
      }

      if ((Link->ForwardLink == &gMenuOption) && (Index <= BottomRow + 1)) {
        //
        // Finally we know that NewPos is the last MenuOption can be focused.
        //
        Repaint = FALSE;
        MoveToNextStatement (TRUE, &Link, Index - TopRow);
      } else {
        //
        // Calculate the skip line for top of screen menu.
        //
        if (Link == TopOfScreen) {
          //
          // The top of screen menu option occupies the entire form.
          //
          SkipValue += BottomRow - TopRow + 1;
        } else {
          SkipValue = NextMenuOption->Skip - (Index - (BottomRow + 1));
        }

        TopOfScreen = Link;
        MenuOption = NULL;
        //
        // Move to the Next selectable menu.
        //
        MoveToNextStatement (FALSE, &Link, BottomRow - TopRow);
      }

      //
      // Save the menu as the next highlight menu.
      //
      NewPos  = Link;

      //
      // If we encounter a Date/Time op-code set, rewind to the first op-code of the set.
      // Don't do this when we are already in the last page.
      //
      AdjustDateAndTimePosition (TRUE, &TopOfScreen);
      AdjustDateAndTimePosition (TRUE, &NewPos);
      break;

    case CfUiDown:
      //
      // SkipValue means lines is skipped when show the top menu option.
      // NewPos  points to the menu which is highlighted now.
      //
      ControlFlag = CfRepaint;

      //
      // Since the behavior of hitting the down arrow on a Date/Time op-code is intended
      // to be one that progresses to the next set of op-codes, we need to advance to the last
      // Date/Time op-code and leave the remaining logic in UiDown intact so the appropriate
      // checking can be done.  The only other logic we need to introduce is that if a Date/Time
      // op-code is the last entry in the menu, we need to rewind back to the first op-code of
      // the Date/Time op-code.
      //
      SavedListEntry = NewPos;
      AdjustDateAndTimePosition (FALSE, &NewPos);

      if (NewPos->ForwardLink != &gMenuOption) {
        MenuOption      = MENU_OPTION_FROM_LINK (NewPos);
        NewLine         = TRUE;
        NewPos          = NewPos->ForwardLink;

        Difference      = 0;
        //
        // Current menu not at the bottom of the form.
        //
        if (BottomRow >= MenuOption->Row + MenuOption->Skip) {
          //
          // Find the next selectable menu.
          //
          Difference = MoveToNextStatement (FALSE, &NewPos, BottomRow - MenuOption->Row - MenuOption->Skip);
          //
          // We hit the end of MenuOption that can be focused
          // so we simply scroll to the first page.
          //
          if (Difference < 0) {
            //
            // Scroll to the first page.
            //
            if (TopOfScreen != gMenuOption.ForwardLink) {
              TopOfScreen = gMenuOption.ForwardLink;
              Repaint     = TRUE;
              MenuOption  = NULL;
            } else {
              MenuOption = MENU_OPTION_FROM_LINK (SavedListEntry);
            }
            NewPos        = gMenuOption.ForwardLink;
            MoveToNextStatement (FALSE, &NewPos, BottomRow - TopRow);

            SkipValue = 0;
            //
            // If we are at the end of the list and sitting on a Date/Time op, rewind to the head.
            //
            AdjustDateAndTimePosition (TRUE, &TopOfScreen);
            AdjustDateAndTimePosition (TRUE, &NewPos);
            break;
          }
        }
        NextMenuOption  = MENU_OPTION_FROM_LINK (NewPos);
        if (NextMenuOption->Row == 0) {
          UpdateOptionSkipLines (NextMenuOption);
        }
        DistanceValue  = Difference + NextMenuOption->Skip;

        Temp = MenuOption->Row + MenuOption->Skip + DistanceValue - 1;
        if ((MenuOption->Row + MenuOption->Skip == BottomRow + 1) &&
            (NextMenuOption->ThisTag->OpCode->OpCode == EFI_IFR_DATE_OP ||
             NextMenuOption->ThisTag->OpCode->OpCode == EFI_IFR_TIME_OP)
            ) {
          Temp ++;
        }

        //
        // If we are going to scroll, update TopOfScreen
        //
        if (Temp > BottomRow) {
          do {
            //
            // Is the current top of screen a zero-advance op-code?
            // If so, keep moving forward till we hit a >0 advance op-code
            //
            SavedMenuOption = MENU_OPTION_FROM_LINK (TopOfScreen);

            //
            // If bottom op-code is more than one line or top op-code is more than one line
            //
            if ((DistanceValue > 1) || (SavedMenuOption->Skip > 1)) {
              //
              // Is the bottom op-code greater than or equal in size to the top op-code?
              //
              if ((Temp - BottomRow) >= (SavedMenuOption->Skip - SkipValue)) {
                //
                // Skip the top op-code
                //
                TopOfScreen     = TopOfScreen->ForwardLink;
                Difference      = (Temp - BottomRow) - (SavedMenuOption->Skip - SkipValue);

                SavedMenuOption = MENU_OPTION_FROM_LINK (TopOfScreen);

                //
                // If we have a remainder, skip that many more op-codes until we drain the remainder
                //
                while (Difference >= (INTN) SavedMenuOption->Skip) {
                  //
                  // Since the Difference is greater than or equal to this op-code's skip value, skip it
                  //
                  Difference      = Difference - (INTN) SavedMenuOption->Skip;
                  TopOfScreen     = TopOfScreen->ForwardLink;
                  SavedMenuOption = MENU_OPTION_FROM_LINK (TopOfScreen);
                }
                //
                // Since we will act on this op-code in the next routine, and increment the
                // SkipValue, set the skips to one less than what is required.
                //
                SkipValue = Difference - 1;
              } else {
                //
                // Since we will act on this op-code in the next routine, and increment the
                // SkipValue, set the skips to one less than what is required.
                //
                SkipValue += (Temp - BottomRow) - 1;
              }
            } else {
              if ((SkipValue + 1) == (INTN) SavedMenuOption->Skip) {
                TopOfScreen = TopOfScreen->ForwardLink;
                break;
              }
            }
            //
            // If the op-code at the top of the screen is more than one line, let's not skip it yet
            // Let's set a skip flag to smoothly scroll the top of the screen.
            //
            if (SavedMenuOption->Skip > 1) {
              if (SavedMenuOption == NextMenuOption) {
                SkipValue = 0;
              } else {
                SkipValue++;
              }
            } else if (SavedMenuOption->Skip == 1) {
              SkipValue   = 0;
            } else {
              SkipValue   = 0;
              TopOfScreen = TopOfScreen->ForwardLink;
            }
          } while (SavedMenuOption->Skip == 0);

          Repaint       = TRUE;
        } else if (!IsSelectable (NextMenuOption)) {
          //
          // Continue to go down until scroll to next page or the selectable option is found.
          //
          ScreenOperation = UiDown;
          ControlFlag     = CfScreenOperation;
        }

        MenuOption = MENU_OPTION_FROM_LINK (SavedListEntry);

        UpdateStatusBar (INPUT_ERROR, FALSE);

      } else {
        //
        // Scroll to the first page.
        //
        if (TopOfScreen != gMenuOption.ForwardLink) {
          TopOfScreen = gMenuOption.ForwardLink;
          Repaint     = TRUE;
          MenuOption  = NULL;
        } else {
          //
          // Need to remove the current highlight menu.
          // MenuOption saved the last highlight menu info.
          //
          MenuOption = MENU_OPTION_FROM_LINK (SavedListEntry);
        }

        SkipValue     = 0;
        NewLine       = TRUE;
        //
        // Get the next highlight menu.
        //
        NewPos        = gMenuOption.ForwardLink;
        MoveToNextStatement (FALSE, &NewPos, BottomRow - TopRow);
      }

      //
      // If we are at the end of the list and sitting on a Date/Time op, rewind to the head.
      //
      AdjustDateAndTimePosition (TRUE, &TopOfScreen);
      AdjustDateAndTimePosition (TRUE, &NewPos);
      break;

    case CfUiNoOperation:
      ControlFlag = CfRepaint;
      break;

    case CfExit:
      if (HelpString != NULL) {
        FreePool (HelpString);
      }
      if (HelpHeaderString != NULL) {
        FreePool (HelpHeaderString);
      }
      if (HelpBottomString != NULL) {
        FreePool (HelpBottomString);
      }
      return EFI_SUCCESS;

    default:
      break;
    }
  }
}

/**

  Base on the browser status info to show an pop up message.

**/
VOID
BrowserStatusProcess (
  VOID
  )
{
  CHAR16         *ErrorInfo;
  EFI_INPUT_KEY  Key;

  if (gFormData->BrowserStatus == BROWSER_SUCCESS) {
    return;
  }

  if (gFormData->ErrorString != NULL) {
    ErrorInfo = gFormData->ErrorString;
  } else {
    switch (gFormData->BrowserStatus) {
    case BROWSER_SUBMIT_FAIL:
      ErrorInfo = gSaveFailed;
      break;

    case BROWSER_NO_SUBMIT_IF:
      ErrorInfo = gNoSubmitIf;
      break;

    case BROWSER_FORM_NOT_FOUND:
      ErrorInfo = gFormNotFound;
      break;

    case BROWSER_FORM_SUPPRESS:
      ErrorInfo = gFormSuppress;
      break;

    case BROWSER_PROTOCOL_NOT_FOUND:
      ErrorInfo = gProtocolNotFound;
      break;

    default:
      ErrorInfo = gBrwoserError;
      break;
    }
  }

  //
  // Error occur, prompt error message.
  //
  do {
    CreateDialog (&Key, gEmptyString, ErrorInfo, gPressEnter, gEmptyString, NULL);
  } while (Key.UnicodeChar != CHAR_CARRIAGE_RETURN);
}

/**
  Display one form, and return user input.
  
  @param FormData                Form Data to be shown.
  @param UserInputData           User input data.
  
  @retval EFI_SUCCESS            1.Form Data is shown, and user input is got.
                                 2.Error info has show and return.
  @retval EFI_INVALID_PARAMETER  The input screen dimension is not valid
  @retval EFI_NOT_FOUND          New form data has some error.
**/
EFI_STATUS
EFIAPI 
FormDisplay (
  IN  FORM_DISPLAY_ENGINE_FORM  *FormData,
  OUT USER_INPUT                *UserInputData
  )
{
  EFI_STATUS  Status;

  ASSERT (FormData != NULL);
  if (FormData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  gUserInput = UserInputData;
  gFormData  = FormData;

  //
  // Process the status info first.
  //
  BrowserStatusProcess();
  if (UserInputData == NULL) {
    //
    // UserInputData == NULL, means only need to print the error info, return here.
    //
    return EFI_SUCCESS;
  }

  ConvertStatementToMenu();

  Status = DisplayPageFrame (FormData, &gStatementDimensions);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (CompareMem (&gOldStatementDimensions, &gStatementDimensions, sizeof (gStatementDimensions)) == 0) {
    mStatementLayoutIsChanged = FALSE;
  } else {
    mStatementLayoutIsChanged = TRUE;
    CopyMem (&gOldStatementDimensions, &gStatementDimensions, sizeof (gStatementDimensions));
  }

  Status = UiDisplayMenu(FormData);

  return Status;
}

/**
  Initialize Setup Browser driver.

  @param ImageHandle     The image handle.
  @param SystemTable     The system table.

  @retval EFI_SUCCESS    The Setup Browser module is initialized correctly..
  @return Other value if failed to initialize the Setup Browser module.

**/
EFI_STATUS
EFIAPI
InitializeDisplayEngine (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                          Status;
  EFI_INPUT_KEY                       HotKey;
  EFI_STRING                          NewString;
  EDKII_FORM_BROWSER_EXTENSION2_PROTOCOL *FormBrowserEx2;

  //
  // Publish our HII data
  //
  gHiiHandle = HiiAddPackages (
                 &gDisplayEngineGuid,
                 ImageHandle,
                 DisplayEngineStrings,
                 NULL
                 );
  ASSERT (gHiiHandle != NULL);

  //
  // Install Form Display protocol
  //
  Status = gBS->InstallProtocolInterface (
                  &mPrivateData.Handle,
                  &gEdkiiFormDisplayEngineProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mPrivateData.FromDisplayProt
                  );
  ASSERT_EFI_ERROR (Status);

  InitializeDisplayStrings();

  //
  // Use BrowserEx2 protocol to register HotKey.
  // 
  Status = gBS->LocateProtocol (&gEdkiiFormBrowserEx2ProtocolGuid, NULL, (VOID **) &FormBrowserEx2);
  if (!EFI_ERROR (Status)) {
    //
    // Register the default HotKey F9 and F10 again.
    //
    HotKey.UnicodeChar = CHAR_NULL;
    HotKey.ScanCode   = SCAN_F10;
    NewString         = HiiGetString (gHiiHandle, STRING_TOKEN (FUNCTION_TEN_STRING), NULL);
    ASSERT (NewString != NULL);
    FormBrowserEx2->RegisterHotKey (&HotKey, BROWSER_ACTION_SUBMIT, 0, NewString);

    HotKey.ScanCode   = SCAN_F9;
    NewString         = HiiGetString (gHiiHandle, STRING_TOKEN (FUNCTION_NINE_STRING), NULL);
    ASSERT (NewString != NULL);
    FormBrowserEx2->RegisterHotKey (&HotKey, BROWSER_ACTION_DEFAULT, EFI_HII_DEFAULT_CLASS_STANDARD, NewString);
  }

  return EFI_SUCCESS;
}

/**
  This is the default unload handle for display core drivers.

  @param[in]  ImageHandle       The drivers' driver image.

  @retval EFI_SUCCESS           The image is unloaded.
  @retval Others                Failed to unload the image.

**/
EFI_STATUS
EFIAPI
UnloadDisplayEngine (
  IN EFI_HANDLE             ImageHandle
  )
{
  HiiRemovePackages(gHiiHandle);

  FreeDisplayStrings ();

  return EFI_SUCCESS;
}