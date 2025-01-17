﻿#include "pch.h"
#include "extension.h"
#include <cwctype>

namespace Extension 
{    
    constexpr wchar_t ERASED = 0xf246;

    void RemoveRepeatChar(std::wstring& sentence)
    {
        std::vector<int> repeatNumbers(sentence.size() + 1, 0);
        for (int i = 0; i < sentence.size(); ++i)
        {
            if (sentence[i] != sentence[i + 1])
            {
                int j = i;
                while (sentence[j] == sentence[i] && --j >= 0);
                repeatNumbers[i - j] += 1;
            }
        }
        int repeatNumber = std::distance(repeatNumbers.begin(), std::max_element(repeatNumbers.rbegin(), repeatNumbers.rend()).base() - 1);
        if (repeatNumber < 2) return;

        std::wstring newSentence;
        for (int i = 0; i < sentence.size();)
        {
            newSentence.push_back(sentence[i]);
            for (int j = i; j <= sentence.size(); ++j)
            {
                if (j == sentence.size() || sentence[i] != sentence[j])
                {
                    i += (j - i) % repeatNumber == 0 ? repeatNumber : 1;
                    break;
                }
            }
        }
        sentence = newSentence;
    }

    std::vector<int> GenerateSuffixArray(const std::wstring& text)
    {
        std::vector<int> suffixArray(text.size());
        for (int i = 0; i < text.size(); ++i) suffixArray[i] = i;
        // The below code is a more efficient way of doing this:
        // std::sort(suffixArray.begin(), suffixArray.end(), [&](int a, int b) { return wcscmp(text.c_str() + a, text.c_str() + b) > 0; });
        std::stable_sort(suffixArray.begin(), suffixArray.end(), [&](int a, int b) { return text[a] > text[b]; });
        std::vector<int> eqClasses(text.begin(), text.end());
        std::vector<int> count(text.size());
        for (int length = 1; length < text.size(); length *= 2)
        {
            // Determine equivalence class up to length, by checking length / 2 equivalence of suffixes and their following length / 2 suffixes
            std::vector<int> prevEqClasses = eqClasses;
            eqClasses[suffixArray[0]] = 0;
            for (int i = 1; i < text.size(); ++i)
            {
                int currentSuffix = suffixArray[i], lastSuffix = suffixArray[i - 1];
                if (currentSuffix + length < text.size() && prevEqClasses[currentSuffix] == prevEqClasses[lastSuffix] &&
                    prevEqClasses[currentSuffix + length / 2] == prevEqClasses[lastSuffix + length / 2])
                    eqClasses[currentSuffix] = eqClasses[lastSuffix];
                else eqClasses[currentSuffix] = i;
            }

            // Sort within equivalence class based on order of following suffix after length (orders up to length * 2)
            for (int i = 0; i < text.size(); ++i) count[i] = i;
            for (auto suffix : std::vector(suffixArray))
            {
                int precedingSuffix = suffix - length;
                if (precedingSuffix >= 0) suffixArray[count[eqClasses[precedingSuffix]]++] = precedingSuffix;
            }
        }
        for (int i = 0; i + 1 < text.size(); ++i)
            assert(wcscmp(text.c_str() + suffixArray[i], text.c_str() + suffixArray[i + 1]) > 0);
        return suffixArray;
    }

    void RemoveRepeatPhrase(std::wstring& sentence)
    {
        // This algorithm looks for repeating substrings (in other words, common prefixes among the set of suffixes) of the sentence with length > 6
        // It then looks for any regions of characters at least twice as long as the substring made up only of characters in the substring, and erases them
        // If this results in the substring being completely erased from the string, the substring is copied to the last location where it was located in the original string
        auto timeout = GetTickCount64() + 30'00; // give up if taking over 3 seconds  3秒足够，再高也是废文本
        std::vector<int> suffixArray = GenerateSuffixArray(sentence);
        for (int i = 0; i + 1 < sentence.size() && GetTickCount64() < timeout; ++i)
        {
            int commonPrefixLength = 0;
            for (int j = suffixArray[i], k = suffixArray[i + 1]; j < sentence.size() && k < sentence.size(); ++j, ++k)
                if (sentence[j] != ERASED && sentence[j] == sentence[k]) commonPrefixLength += 1;
                else break;

            if (commonPrefixLength > 6)
            {
                std::wstring substring(sentence, suffixArray[i], commonPrefixLength);
                bool substringCharMap[0x10000] = {};
                for (auto ch : substring)
                    substringCharMap[ch] = true;

                for (int regionSize = 0, j = 0; j <= sentence.size(); ++j)
                    if (substringCharMap[sentence[j]]) regionSize += 1;
                    else if (regionSize >= commonPrefixLength * 2)
                        while (regionSize > 0)
                            sentence[j - regionSize--] = ERASED;
                    else regionSize = 0;

                if (!wcsstr(sentence.c_str(), substring.c_str())) std::copy(substring.begin(), substring.end(), sentence.begin() + max(suffixArray[i], suffixArray[i + 1]));
            }
        }
        sentence.erase(std::remove(sentence.begin(), sentence.end(), ERASED), sentence.end());
    }

    template <typename Sequence, typename Pred>     
    Sequence& trim(Sequence& seq, Pred pred) 
    {
        return trim_start(trim_end(seq, pred), pred);
    }

    template <typename Sequence, typename Pred>
    Sequence& trim_end(Sequence& seq, Pred pred) 
    {
        auto last = std::find_if_not(seq.rbegin(), seq.rend(), pred);
        seq.erase(last.base(), seq.end());
        return seq;
    }

    template <typename Sequence, typename Pred>
    Sequence& trim_start(Sequence& seq, Pred pred) 
    {
        auto first = std::find_if_not(seq.begin(), seq.end(), pred);
        seq.erase(seq.begin(), first);
        return seq;
    }

    std::wstring& trim(std::wstring& str) 
    {
        return trim(str, [](const wchar_t c) { return std::iswspace(c); });
    }

    std::wstring& trim_start(std::wstring& str) 
    {
        return trim_start(str, [](const wchar_t c) { return std::iswspace(c); });
    }

    std::wstring& trim_end(std::wstring& str) 
    {
        return trim_end(str, [](const wchar_t c) { return std::iswspace(c); });
    }
}