/***************************************************************************
                  russian_stem.h  -  stemmer for Russian text.
                             -------------------
    begin                : December 2010
    copyright            : (C) 2010 by Blake Madden
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License.                                *
 *                                                                         *
 ***************************************************************************/

#ifndef __RUSSIAN_STEM_H__
#define __RUSSIAN_STEM_H__

#include "stemming.h"

namespace stemming
    {
    static const wchar_t RUSSIAN_VOWELS[] = { 0x0410, 0x0430, 0x0415, 0x0435, 0x0418, 0x0438,
                                              0x041E, 0x043E, 0x423, 0x0443, 0x042B, 0x044B,
                                              0x042D, 0x044D, 0x042E, 0x044E, 0x042F, 0x044F, 0 };

    static const wchar_t RUSSIAN_A_UPPER = 0x0410;
    static const wchar_t RUSSIAN_A_LOWER = 0x0430;

    static const wchar_t RUSSIAN_BE_UPPER = 0x0411;
    static const wchar_t RUSSIAN_BE_LOWER = 0x0431;

    static const wchar_t RUSSIAN_VE_UPPER = 0x0412;
    static const wchar_t RUSSIAN_VE_LOWER = 0x0432;

    static const wchar_t RUSSIAN_GHE_UPPER = 0x0413;
    static const wchar_t RUSSIAN_GHE_LOWER = 0x0433;

    static const wchar_t RUSSIAN_DE_UPPER = 0x0414;
    static const wchar_t RUSSIAN_DE_LOWER = 0x0434;

    static const wchar_t RUSSIAN_IE_UPPER = 0x0415;
    static const wchar_t RUSSIAN_IE_LOWER = 0x0435;

    static const wchar_t RUSSIAN_ZHE_UPPER = 0x0416;
    static const wchar_t RUSSIAN_ZHE_LOWER = 0x0436;

    static const wchar_t RUSSIAN_ZE_UPPER = 0x0417;
    static const wchar_t RUSSIAN_ZE_LOWER = 0x0437;

    static const wchar_t RUSSIAN_I_UPPER = 0x0418;
    static const wchar_t RUSSIAN_I_LOWER = 0x0438;

    static const wchar_t RUSSIAN_SHORT_I_UPPER = 0x0419;
    static const wchar_t RUSSIAN_SHORT_I_LOWER = 0x0439;

    static const wchar_t RUSSIAN_KA_UPPER = 0x041A;
    static const wchar_t RUSSIAN_KA_LOWER = 0x043A;

    static const wchar_t RUSSIAN_EL_UPPER = 0x041B;
    static const wchar_t RUSSIAN_EL_LOWER = 0x043B;

    static const wchar_t RUSSIAN_EM_UPPER = 0x041C;
    static const wchar_t RUSSIAN_EM_LOWER = 0x043C;

    static const wchar_t RUSSIAN_EN_UPPER = 0x041D;
    static const wchar_t RUSSIAN_EN_LOWER = 0x043D;

    static const wchar_t RUSSIAN_O_UPPER = 0x041E;
    static const wchar_t RUSSIAN_O_LOWER = 0x043E;

    static const wchar_t RUSSIAN_PE_UPPER = 0x041F;
    static const wchar_t RUSSIAN_PE_LOWER = 0x043F;

    static const wchar_t RUSSIAN_ER_UPPER = 0x0420;
    static const wchar_t RUSSIAN_ER_LOWER = 0x0440;

    static const wchar_t RUSSIAN_ES_UPPER = 0x0421;
    static const wchar_t RUSSIAN_ES_LOWER = 0x0441;

    static const wchar_t RUSSIAN_TE_UPPER = 0x0422;
    static const wchar_t RUSSIAN_TE_LOWER = 0x0442;

    static const wchar_t RUSSIAN_U_UPPER = 0x0423;
    static const wchar_t RUSSIAN_U_LOWER = 0x0443;

    static const wchar_t RUSSIAN_EF_UPPER = 0x0424;
    static const wchar_t RUSSIAN_EF_LOWER = 0x0444;

    static const wchar_t RUSSIAN_HA_UPPER = 0x0425;
    static const wchar_t RUSSIAN_HA_LOWER = 0x0445;

    static const wchar_t RUSSIAN_TSE_UPPER = 0x0426;
    static const wchar_t RUSSIAN_TSE_LOWER = 0x0446;

    static const wchar_t RUSSIAN_CHE_UPPER = 0x0427;
    static const wchar_t RUSSIAN_CHE_LOWER = 0x0447;

    static const wchar_t RUSSIAN_SHA_UPPER = 0x0428;
    static const wchar_t RUSSIAN_SHA_LOWER = 0x0448;

    static const wchar_t RUSSIAN_SHCHA_UPPER = 0x0429;
    static const wchar_t RUSSIAN_SHCHA_LOWER = 0x0449;

    static const wchar_t RUSSIAN_HARD_SIGN_UPPER = 0x042A;
    static const wchar_t RUSSIAN_HARD_SIGN_LOWER = 0x044A;

    static const wchar_t RUSSIAN_YERU_UPPER = 0x042B;
    static const wchar_t RUSSIAN_YERU_LOWER = 0x044B;

    static const wchar_t RUSSIAN_SOFT_SIGN_UPPER = 0x042C;
    static const wchar_t RUSSIAN_SOFT_SIGN_LOWER = 0x044C;

    static const wchar_t RUSSIAN_E_UPPER = 0x042D;
    static const wchar_t RUSSIAN_E_LOWER = 0x044D;

    static const wchar_t RUSSIAN_YU_UPPER = 0x042E;
    static const wchar_t RUSSIAN_YU_LOWER = 0x044E;

    static const wchar_t RUSSIAN_YA_UPPER = 0x042F;
    static const wchar_t RUSSIAN_YA_LOWER = 0x044F;

    /**@htmlonly
    PERFECTIVE GERUND:<br />
    group 1:   &#1074; &#1074;&#1096;&#1080; &#1074;&#1096;&#1080;&#1089;&#1100;<br />
    group 2:   &#1080;&#1074; &#1080;&#1074;&#1096;&#1080; &#1080;&#1074;&#1096;&#1080;&#1089;&#1100; &#1099;&#1074; &#1099;&#1074;&#1096;&#1080; &#1099;&#1074;&#1096;&#1080;&#1089;&#1100;<br /><br />
    group 1 endings must follow &#1072; or &#1103;.<br /><br />
    
    REFLEXIVE:<br />
    &#1089;&#1103; &#1089;&#1100;<br /><br />

    NOUN:<br />
    &#1072; &#1077;&#1074; &#1086;&#1074; &#1080;&#1077; &#1100;&#1077; &#1077; &#1080;&#1103;&#1084;&#1080;<br />
    &#1103;&#1084;&#1080; &#1072;&#1084;&#1080; &#1077;&#1080; &#1080;&#1080; &#1080; &#1080;&#1077;&#1081;<br />
    &#1077;&#1081; &#1086;&#1081; &#1080;&#1081; &#1081; &#1080;&#1103;&#1084; &#1103;&#1084; &#1080;&#1077;&#1084;<br />
    &#1077;&#1084; &#1072;&#1084; &#1086;&#1084; &#1086; &#1091; &#1072;&#1093; &#1080;&#1103;&#1093; &#1103;&#1093; &#1099;<br />
    &#1100; &#1080;&#1102; &#1100;&#1102; &#1102; &#1080;&#1103; &#1100;&#1103; &#1103;
    @endhtmlonly*/
    template <typename string_typeT = std::wstring>
    class russian_stem : public stem<string_typeT>
        {
    public:
        //---------------------------------------------
        /**@param text string to stem*/
        void operator()(string_typeT& text)
            {
            if (text.length() < 2)
                { return; }

            //reset internal data
            stem<string_typeT>::reset_r_values();

            this->find_r1(text, RUSSIAN_VOWELS);
            this->find_r2(text, RUSSIAN_VOWELS);
            this->find_russian_rv(text, RUSSIAN_VOWELS);

            //change 33rd letter ('&#x0451;') to '&#x0435;'
            for (size_t i = 0; i < text.length(); ++i)
                {
                if (text[i] == 0x0451)
                    { text[i] = RUSSIAN_IE_LOWER; }
                else if (text[i] == 0x0401)
                    { text[i] = RUSSIAN_IE_UPPER; }
                }

            step_1(text);
            step_2(text);
            step_3(text);
            step_4(text);
            }
    private:
        /**@htmlonly
        Step 1: Search for a PERFECTIVE GERUND ending. If one is found remove it,
        and that is then the end of step 1. Otherwise try and remove a REFLEXIVE ending,
        and then search in turn for<br />
        (1) an ADJECTIVAL,<br />
        (2) a VERB or<br />
        (3) a NOUN ending.<br />
        As soon as one of the endings (1) to (3) is found remove it, and terminate step 1.
        @endhtmlonly*/
        void step_1(string_typeT& text)
            {
            //search for a perfect gerund
            //group 2
            if (this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER,
                                   RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER,
                                   RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER,
                                   RUSSIAN_I_LOWER, RUSSIAN_I_UPPER,
                                   RUSSIAN_ES_LOWER, RUSSIAN_ES_UPPER,
                                   RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER,
                                   RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER,
                                   RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER,
                                   RUSSIAN_I_LOWER, RUSSIAN_I_UPPER,
                                   RUSSIAN_ES_LOWER, RUSSIAN_ES_UPPER,
                                   RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER, false) )
                { return; }
            //group 1
            else if (this->is_suffix_in_rv(text, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER,
                                        RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER,
                                        RUSSIAN_I_LOWER, RUSSIAN_I_UPPER,
                                        RUSSIAN_ES_LOWER, RUSSIAN_ES_UPPER,
                                        RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER) )
                {
                if (this->is_suffix_in_rv(text, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER,
                                    RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER,
                                    RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER,
                                    RUSSIAN_I_LOWER, RUSSIAN_I_UPPER,
                                    RUSSIAN_ES_LOWER, RUSSIAN_ES_UPPER,
                                    RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER) ||
                    this->is_suffix_in_rv(text, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER,
                                    RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER,
                                    RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER,
                                    RUSSIAN_I_LOWER, RUSSIAN_I_UPPER,
                                    RUSSIAN_ES_LOWER, RUSSIAN_ES_UPPER,
                                    RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER) )
                    {
                    text.erase(text.end()-5, text.end() );
                    this->update_r_sections(text);
                    return;
                    }
                }
            //group 2
            else if (this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER,
                                        RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER,
                                        RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER,
                                        RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) ||
                     this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER,
                                        RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER,
                                        RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER,
                                        RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) )
                { return; }
            //group 1
            else if (this->is_suffix_in_rv(text, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER,
                                        RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER,
                                        RUSSIAN_I_LOWER, RUSSIAN_I_UPPER) )
                {
                if (this->is_suffix_in_rv(text, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER,
                                    RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER,
                                    RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER,
                                    RUSSIAN_I_LOWER, RUSSIAN_I_UPPER) ||
                    this->is_suffix_in_rv(text, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER,
                                    RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER,
                                    RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER,
                                    RUSSIAN_I_LOWER, RUSSIAN_I_UPPER) )
                    {
                    text.erase(text.end()-3, text.end() );
                    this->update_r_sections(text);
                    return;
                    }
                }
            //group 2
            else if (this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER, false) )
                { return; }
            //group 1
            else if (this->is_suffix_in_rv(text, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER) )
                {
                if (this->is_suffix_in_rv(text, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER) ||
                    this->is_suffix_in_rv(text, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER) )
                    {
                    text.erase(text.end()-1, text.end() );
                    this->update_r_sections(text);
                    return;
                    }
                }
            //reflexive
            if (this->delete_if_is_in_rv(text, RUSSIAN_ES_LOWER, RUSSIAN_ES_UPPER, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_ES_LOWER, RUSSIAN_ES_UPPER, RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER, false) )
                { /*NOOP*/ }
            //adjectival
            if (this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_UPPER, RUSSIAN_YERU_LOWER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_GHE_LOWER, RUSSIAN_GHE_UPPER, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, RUSSIAN_GHE_LOWER, RUSSIAN_GHE_UPPER, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, RUSSIAN_U_LOWER, RUSSIAN_U_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, RUSSIAN_U_LOWER, RUSSIAN_U_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, false) ||                
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, false) ||                
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, false) ||                
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_HA_LOWER, RUSSIAN_HA_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_HA_LOWER, RUSSIAN_HA_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_U_LOWER, RUSSIAN_U_UPPER, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, false) )
                {
                //delete participles
                //group 2
                if (this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER, RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER, false) ||
                    this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER, RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER, false) ||
                    this->delete_if_is_in_rv(text, RUSSIAN_U_LOWER, RUSSIAN_U_UPPER, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, RUSSIAN_SHCHA_LOWER, RUSSIAN_SHCHA_UPPER, false) )
                    {/*NOOP*/}
                //group 1
                else if (this->is_suffix_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER) ||
                        this->is_suffix_in_rv(text, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER) ||
                        this->is_suffix_in_rv(text, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER, RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER) ||
                        this->is_suffix_in_rv(text, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, RUSSIAN_SHCHA_LOWER, RUSSIAN_SHCHA_UPPER) )
                    {
                    if (text.length() >= 3 &&
                        stem<string_typeT>::get_rv() <= text.length()-3 &&
                        (is_either<wchar_t>(text[text.length()-3], RUSSIAN_A_LOWER, RUSSIAN_A_UPPER) ||
                        is_either<wchar_t>(text[text.length()-3], RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER)) )
                        {
                        text.erase(text.end()-2, text.end() );
                        this->update_r_sections(text);
                        return;
                        }
                    }
                else if (this->is_suffix_in_rv(text, RUSSIAN_SHCHA_LOWER, RUSSIAN_SHCHA_UPPER) )
                    {
                    if (text.length() >= 2 &&
                        stem<string_typeT>::get_rv() <= text.length()-2 &&
                        (is_either<wchar_t>(text[text.length()-2], RUSSIAN_A_LOWER, RUSSIAN_A_UPPER) ||
                        is_either<wchar_t>(text[text.length()-2], RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER)) )
                        {
                        text.erase(text.end()-1, text.end() );
                        this->update_r_sections(text);
                        return;
                        }
                    }
                return;
                }
            //verb
            //group 2
            else if (this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, false) ||/*4*/
                this->delete_if_is_in_rv(text, RUSSIAN_U_LOWER, RUSSIAN_U_UPPER, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_U_LOWER, RUSSIAN_U_UPPER, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, false) )
                {
                return;
                }
            //group 1
            if (this->is_suffix_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER) ||
                this->is_suffix_in_rv(text, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER) ||
                this->is_suffix_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER, RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER) ||
                this->is_suffix_in_rv(text, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER) )
                {
                if (text.length() >= 4 &&
                    stem<string_typeT>::get_rv() <= text.length()-4 &&
                    (is_either<wchar_t>(text[text.length()-4], RUSSIAN_A_LOWER, RUSSIAN_A_UPPER) ||
                    is_either<wchar_t>(text[text.length()-4], RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER)) )
                    {
                    text.erase(text.end()-3, text.end() );
                    this->update_r_sections(text);
                    return;
                    }
                }
            //group 2
            else if (this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_EL_LOWER, RUSSIAN_EL_UPPER, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_EL_LOWER, RUSSIAN_EL_UPPER, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_EL_LOWER, RUSSIAN_EL_UPPER, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_EL_LOWER, RUSSIAN_EL_UPPER, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_EL_LOWER, RUSSIAN_EL_UPPER, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_EL_LOWER, RUSSIAN_EL_UPPER, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_U_LOWER, RUSSIAN_U_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_U_LOWER, RUSSIAN_U_UPPER, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER, RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_EL_LOWER, RUSSIAN_EL_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_EL_LOWER, RUSSIAN_EL_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER, false) ||                
                this->delete_if_is_in_rv(text, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, false) ||                
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, false) ||                
                this->delete_if_is_in_rv(text, RUSSIAN_U_LOWER, RUSSIAN_U_UPPER, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, false) )
                {
                return;
                }
            //group 1
            else if (this->is_suffix_in_rv(text, RUSSIAN_EL_LOWER, RUSSIAN_EL_UPPER, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER) ||/*2*/
                this->is_suffix_in_rv(text, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER) ||
                this->is_suffix_in_rv(text, RUSSIAN_EL_LOWER, RUSSIAN_EL_UPPER, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER) ||
                this->is_suffix_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER) ||
                this->is_suffix_in_rv(text, RUSSIAN_EL_LOWER, RUSSIAN_EL_UPPER, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER) ||
                this->is_suffix_in_rv(text, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER) ||
                this->is_suffix_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER) ||
                this->is_suffix_in_rv(text, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER) ||
                this->is_suffix_in_rv(text, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER) ||
                this->is_suffix_in_rv(text, RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER, RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER) )
                {
                if (text.length() >= 3 &&
                    stem<string_typeT>::get_rv() <= text.length()-3 &&
                    (is_either<wchar_t>(text[text.length()-3], RUSSIAN_A_LOWER, RUSSIAN_A_UPPER) ||
                    is_either<wchar_t>(text[text.length()-3], RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER)) )
                    {
                    text.erase(text.end()-2, text.end() );
                    this->update_r_sections(text);
                    return;
                    }
                }
            if (this->is_suffix_in_rv(text, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER) ||/*1*/
                this->is_suffix_in_rv(text, RUSSIAN_EL_LOWER, RUSSIAN_EL_UPPER) ||
                this->is_suffix_in_rv(text, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER) )
                {
                if (text.length() >= 2 &&
                    stem<string_typeT>::get_rv() <= text.length()-2 &&
                    (is_either<wchar_t>(text[text.length()-2], RUSSIAN_A_LOWER, RUSSIAN_A_UPPER) ||
                    is_either<wchar_t>(text[text.length()-2], RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER)) )
                    {
                    text.erase(text.end()-1, text.end() );
                    this->update_r_sections(text);
                    return;
                    }
                }

            //noun
            if (this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER,
                                   RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER,
                                   RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER,
                                   RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, RUSSIAN_HA_LOWER, RUSSIAN_HA_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER, false) ||                
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_VE_LOWER, RUSSIAN_VE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER, RUSSIAN_HA_LOWER, RUSSIAN_HA_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, RUSSIAN_EM_LOWER, RUSSIAN_EM_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, RUSSIAN_HA_LOWER, RUSSIAN_HA_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_I_LOWER, RUSSIAN_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_A_LOWER, RUSSIAN_A_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_U_LOWER, RUSSIAN_U_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YU_LOWER, RUSSIAN_YU_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YA_LOWER, RUSSIAN_YA_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_YERU_LOWER, RUSSIAN_YERU_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER, false) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER, false) )
                { return; }
            }
        /**@htmlonly If the word ends with &#1080;, then remove it.@endhtmlonly*/
        void step_2(string_typeT& text)
            { this->delete_if_is_in_rv(text, RUSSIAN_I_UPPER, RUSSIAN_I_LOWER); }
        /**@htmlonly
           Search for a DERIVATIONAL (&#x043e;&#x0441;&#x0442;, &#x043e;&#x0441;&#x0442;&#x044c;)
           ending in R2 (i.e., the entire ending must lie in R2), and if one is found, then remove it.
           @endhtmlonly*/
        void step_3(string_typeT& text)
            {
            if (this->is_suffix_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER,
                                   RUSSIAN_ES_LOWER, RUSSIAN_ES_UPPER,
                                   RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER,
                                   RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER) )
                {
                this->delete_if_is_in_r2(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER,
                                   RUSSIAN_ES_LOWER, RUSSIAN_ES_UPPER,
                                   RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER,
                                   RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER);
                }
            else if (this->is_suffix_in_rv(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER,
                                RUSSIAN_ES_LOWER, RUSSIAN_ES_UPPER,
                                RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER) )
                {
                this->delete_if_is_in_r2(text, RUSSIAN_O_LOWER, RUSSIAN_O_UPPER,
                                   RUSSIAN_ES_LOWER, RUSSIAN_ES_UPPER,
                                   RUSSIAN_TE_LOWER, RUSSIAN_TE_UPPER);
                }
            }
        /**@htmlonly
           (1) Undouble &#1085;, or<br />
           (2) if the word ends with a SUPERLATIVE (&#1077;&#1081;&#1096; or &#1077;&#1081;&#1096;&#1077;) ending, remove it and undouble &#1085;, or<br />
           (3) if the word ends &#1100; then remove it.
           @endhtmlonly*/
        void step_4(string_typeT& text)
            {
            if (this->is_suffix_in_rv(text, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER,
                                RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER) )
                {
                text.erase(text.end()-1, text.end() );
                this->update_r_sections(text);
                }
            else if (this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER,
                                          RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER,
                                          RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER,
                                          RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER) ||
                this->delete_if_is_in_rv(text, RUSSIAN_IE_LOWER, RUSSIAN_IE_UPPER,
                                          RUSSIAN_SHORT_I_LOWER, RUSSIAN_SHORT_I_UPPER,
                                          RUSSIAN_SHA_LOWER, RUSSIAN_SHA_UPPER))
                {
                if (this->is_suffix_in_rv(text, RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER,
                                RUSSIAN_EN_LOWER, RUSSIAN_EN_UPPER) )
                    {
                    text.erase(text.end()-1, text.end() );
                    this->update_r_sections(text);
                    }
                }
            else
                { this->delete_if_is_in_rv(text, RUSSIAN_SOFT_SIGN_LOWER, RUSSIAN_SOFT_SIGN_UPPER); }
            }
        };
    }

#endif //__RUSSIAN_STEM_H__
