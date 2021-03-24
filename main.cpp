#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <boost/algorithm/string.hpp>

enum class EObjectType
{
  INT,
  STRING,
  ARRAY
};

enum class ETokenType
{
  OPEN_BRACKET,
  CLOSE_BRACKET,
  INT,
  STRING,
  COMMA,
  OTHER
};

class CObject
{
  public:
    CObject get(int idx, int pathNum)
    {
      try
      {
        return *((this->items).at(idx));
      }
      catch(std::exception e)
      {
        std::cerr << "ISAN PATH ERROR: " << pathNum;
        exit(-3);
      }
    }

    EObjectType type;
    std::string value;
    std::vector<std::shared_ptr<CObject>> items;
};
    
class CToken
{   
  public:
    CToken(ETokenType typ, std::string val, int start, int end):
    type(typ), value(val), startPos(start), endPos(end)
    {
    }

    ETokenType type{ETokenType::OTHER};
    std::string value;
    int startPos;
    int endPos;
};

CObject parseFromTokens( std::vector<CToken> tokens)
{
  CObject obj;
  if(tokens.size() == 1)
  {
    switch(tokens.front().type)
    {
      case ETokenType::COMMA:
      case ETokenType::OPEN_BRACKET:
      case ETokenType::CLOSE_BRACKET:
      {
        std::cerr << "ISAN SYNTAX ERROR: " << tokens.front().startPos;
        exit(-1);
      }
      break;
      case ETokenType::INT:
      {
        obj.type = EObjectType::INT;
        obj.value = tokens[0].value;
        return obj;
      }
      break;
      case ETokenType::STRING:
      {
        obj.type = EObjectType::STRING;
        obj.value = tokens[0].value;
        return obj;
      }

    }
  }

  if(tokens.size() == 2)
  {
    if(tokens.front().type == ETokenType::OPEN_BRACKET &&
       tokens.back().type == ETokenType::CLOSE_BRACKET)
    {
      obj.type = EObjectType::ARRAY;
      return obj;
    }
    else
    {
      std::cerr << "ISAN SYNTAX ERROR: " << tokens[0].startPos << std::endl;
      exit(-1);
    }
  }

  if(tokens.size() > 2)
  {
    obj.type = EObjectType::ARRAY;

    //opening/closing brackets
    if((tokens.front().type != ETokenType::OPEN_BRACKET) && (tokens.back().type != ETokenType::CLOSE_BRACKET))
    {
      std::cerr << "ISAN SYNTAX ERROR: " << tokens.front().startPos;
      exit(-3); 
    }

    //checking for repetitive commas
    for(auto i = 1; i < tokens.size()-2; i++)
    {
      if(tokens[i].type == ETokenType::COMMA && tokens[i+1].type == ETokenType::COMMA)
      {
        std::cerr << "ISAN SYNTAX ERROR: " << tokens[i].startPos;
        exit(-3);
      }
    }

    //remove commas
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(),[](CToken tok) {
        return tok.type == ETokenType::COMMA;
    }), tokens.end());

    //checking for correct bracket seq
    int counter = 0;
    for(auto i = 1; i < tokens.size()-1; i++)
    {
      if(tokens[i].type == ETokenType::OPEN_BRACKET)
        counter++;
      if(tokens[i].type == ETokenType::CLOSE_BRACKET)
        counter--;
    }
    if(counter!=0)
    {
      std::cerr << "ISAN SYNTAX ERROR: 0";//todo 
      exit(-3);
    }

    //splitting and recursice parsing
    counter = 0;
    auto recStart = 1;
    for(auto i = 1; i < tokens.size()-1; i++)
    {
      if(tokens[i].type == ETokenType::OPEN_BRACKET)
        counter++;
      if(tokens[i].type == ETokenType::CLOSE_BRACKET)
        counter--;
      if(counter == 0)
      {
          std::vector<CToken> tmpVec(tokens.cbegin() + recStart, tokens.cbegin() + i + 1);
          auto tmpObject = parseFromTokens(tmpVec);
          obj.items.push_back(std::make_shared<CObject>(tmpObject));
          recStart = i+1;
      }
    }
  }
  return obj;
}

int main(int argc, char ** argv)
{
    
    std::string isanString;
    std::getline(std::cin, isanString);
    //var declaration
    bool isTypeRequested = false;
    bool isValueRequested = false;
    std::vector<int> elementPath;
    std::vector<CToken> tokens;
    CObject parsedObject;

    if(argc != 2)
    {
        std::cerr << "ISAN COMMAND LINE ARGUMENTS ERROR("<< argc << " arguments provided but 2 required)";
        exit(-1);
    }

    //check elementPath validity
    std::string elPathString(argv[1]);
    std::vector<std::string> tempPath;
    
    switch(elPathString.back())
    {
      case 't':
        isTypeRequested = true;
      break;
      case 'v':
        isValueRequested = true;
      break;
      default:
        std::cerr << "ISAN COMMAND LINE ARGUMENTS ERROR(no type/value specifier at the end of elementPath provided)";
        exit(-1);
    }

    elPathString = elPathString.substr(0, elPathString.length()-1);

    tempPath = boost::split(tempPath, 
    elPathString, boost::is_any_of("."));
    for(auto& chunk : tempPath)
    {
      try
      {
        auto num = std::stoi(chunk);
        if(num >= 0)
          elementPath.push_back(num);
        else
        {
          std::cerr << "ISAN COMMAND LINE ARGUMENTS ERROR(no negative ints in elementPath allowed)";
          exit(-1);
        }
      }
      catch(std::exception& e)
      {
        std::cerr << "ISAN COMMAND LINE ARGUMENTS ERROR:" << chunk << "is not an integer";
        exit(-1);
      }
    }
    ///////////////////

    ///let's split source string into tokens
    size_t idx = 0;
    while(idx < isanString.length() )
    {
      if(isanString[idx] == '[')
      {
        CToken tok(ETokenType::OPEN_BRACKET, "[", idx, idx);
        tokens.push_back(tok);
        idx++;
        continue;
      }
      if(isanString[idx] == ']')
      {
        CToken tok(ETokenType::CLOSE_BRACKET, "]", idx, idx);
        tokens.push_back(tok);
        idx++;
        continue;
      }
      if(isanString[idx] == ',')
      {
        CToken tok(ETokenType::COMMA, ",", idx, idx);
        tokens.push_back(tok);
        idx++;
        continue;
      }
      if(isanString[idx] == '"')
      {
        auto j = idx+1;
        while(j < isanString.length() && isanString[j] != '"' )
          j++;
        if(j == isanString.length())
        {
          std::cerr << "ISAN SYNTAX ERROR: " << idx+1 << "(Unbalanced or unexpected quote)";
          exit(-2);
        }
        else
        {
          CToken tok(ETokenType::STRING, isanString.substr(idx+1, j-idx-1), idx+1, j-idx-1);
          tokens.push_back(tok);
          idx = j+1;
          continue;
        }
      }
      if((isanString[idx] <= '9' && isanString[idx] >= '0') || isanString[idx] == '-')//seeking for integer
      {
        auto j = idx;
        if(isanString[j] == '-')
          j++;
        while(j < isanString.length() && isdigit(isanString[j]))
          j++;
        CToken tok(ETokenType::INT, isanString.substr(idx, j-idx), idx, j-idx);
        tokens.push_back(tok);
        idx = j;
        continue;
      }
      if(isspace(isanString[idx]))
      {
        idx++;
        continue;
      }
      //for symbols not catched above
        std::cerr << "ISAN SYNTAX ERROR: " << idx << "(unrecognized symbol: " << isanString[idx] << ")";
        exit(-2);
      
    }
    /////////////////////////////////////////////////////////

    ///parse from tokens
    parsedObject = parseFromTokens(tokens);
    if(parsedObject.type != EObjectType::ARRAY)
    {
      std::cerr << "ISAN SYNTAX ERROR: 0(high level object is not an array";
      exit(-2);
    }
    ///////

    //elementPath
    auto element = parsedObject;
    for(auto idx = 0; idx < elementPath.size(); idx++)
      element = element.get(elementPath[idx], idx);
    ////

    //type/value
    if(true == isTypeRequested)
    {
      switch(element.type)
      {
        case EObjectType::ARRAY:
          std::cout << "array";
        break;
        case EObjectType::INT:
          std::cout << "int";
        break;
        case EObjectType::STRING:
          std::cout << "string";
        break;
      }
    }

    if(true == isValueRequested)
    {
      if(element.type == EObjectType::ARRAY)
        std::cout << "array";
      else
        std::cout << element.value;
    }
    //////

    return 0;
}