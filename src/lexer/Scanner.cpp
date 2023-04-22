#include "Scanner.h"

using namespace minisolc;

void Scanner::next() {
    if (m_eof) return;
    TokenDesc tokenDesc;
    

    if (m_token[Current].token == Token::EOS) m_eof = true;
}