#ifndef COMBINATORS_H
#define COMBINATORS_H

#include "Parser.h"
#include <vector>
#include <tuple>
#include <utility>

// 组合器：seq - 顺序组合多个解析器
template <typename... Parsers>
struct SeqParser;

template <typename Parser>
struct SeqParser<Parser> {
    using ResultType = decltype(std::declval<Parser>()(string_view{}).value().first);
    
    static Parser<ResultType> create(Parser&& parser) {
        return std::forward<Parser>(parser);
    }
};

template <typename FirstParser, typename... RestParsers>
struct SeqParser<FirstParser, RestParsers...> {
    using FirstResult = decltype(std::declval<FirstParser>()(string_view{}).value().first);
    using RestResult = decltype(std::declval<typename SeqParser<RestParsers...>::ResultType>()());
    using ResultType = decltype(std::tuple_cat(
        std::make_tuple(std::declval<FirstResult>()),
        std::declval<RestResult>()
    ));
    
    static Parser<ResultType> create(FirstParser&& first, RestParsers&&... rest) {
        auto rest_parser = SeqParser<RestParsers...>::create(std::forward<RestParsers>(rest)...);
        
        return PARSER_LAMBDA {
            auto first_result = first(input);
            if (!first_result) {
                return Optional<std::pair<ResultType, string_view>>();
            }
            
            auto rest_result = rest_parser(first_result->second);
            if (!rest_result) {
                return Optional<std::pair<ResultType, string_view>>();
            }
            
            return Optional<std::pair<ResultType, string_view>>(std::make_pair(
                std::tuple_cat(
                    std::make_tuple(first_result->first),
                    rest_result->first
                ),
                rest_result->second
            ));
        };
    }
};

template <typename... Parsers>
auto seq(Parsers&&... parsers) {
    return SeqParser<Parsers...>::create(std::forward<Parsers>(parsers)...);
}

// 组合器：alt - 选择组合多个解析器
template <typename Parser>
Parser alt(Parser&& parser) {
    return std::forward<Parser>(parser);
}

template <typename FirstParser, typename... RestParsers>
auto alt(FirstParser&& first, RestParsers&&... rest) {
    auto rest_alt = alt(std::forward<RestParsers>(rest)...);
    
    return PARSER_LAMBDA {
        auto first_result = first(input);
        if (first_result) {
            return first_result;
        }
        return rest_alt(input);
    };
}

// 组合器：many0 - 匹配0次或多次
template <typename Parser>
Parser<std::vector<typename Parser::result_type::value_type::first_type>> many0(Parser&& parser) {
    using ResultType = typename Parser::result_type::value_type::first_type;
    
    return PARSER_LAMBDA {
        std::vector<ResultType> results;
        string_view current_input = input;
        
        while (true) {
            auto result = parser(current_input);
            if (!result) {
                break;
            }
            results.push_back(result->first);
            current_input = result->second;
        }
        
        return Optional<std::pair<std::vector<ResultType>, string_view>>(std::make_pair(results, current_input));
    };
}

// 组合器：many1 - 匹配1次或多次
template <typename Parser>
Parser<std::vector<typename Parser::result_type::value_type::first_type>> many1(Parser&& parser) {
    using ResultType = typename Parser::result_type::value_type::first_type;
    
    return PARSER_LAMBDA {
        std::vector<ResultType> results;
        string_view current_input = input;
        
        auto first_result = parser(current_input);
        if (!first_result) {
            return Optional<std::pair<std::vector<ResultType>, string_view>>();
        }
        results.push_back(first_result->first);
        current_input = first_result->second;
        
        while (true) {
            auto result = parser(current_input);
            if (!result) {
                break;
            }
            results.push_back(result->first);
            current_input = result->second;
        }
        
        return Optional<std::pair<std::vector<ResultType>, string_view>>(std::make_pair(results, current_input));
    };
}

// 组合器：separated_list - 匹配由分隔符分隔的列表
template <typename Parser, typename SeparatorParser>
Parser<std::vector<typename Parser::result_type::value_type::first_type>> 
separated_list(Parser&& parser, SeparatorParser&& separator) {
    using ResultType = typename Parser::result_type::value_type::first_type;
    
    return PARSER_LAMBDA {
        std::vector<ResultType> results;
        string_view current_input = input;
        
        // 匹配第一个元素
        auto first_result = parser(current_input);
        if (!first_result) {
            // 没有元素，返回空列表
            return Optional<std::pair<std::vector<ResultType>, string_view>>(std::make_pair(results, current_input));
        }
        results.push_back(first_result->first);
        current_input = first_result->second;
        
        // 匹配分隔符和后续元素
        while (true) {
            auto sep_result = separator(current_input);
            if (!sep_result) {
                break;
            }
            current_input = sep_result->second;
            
            auto elem_result = parser(current_input);
            if (!elem_result) {
                // 分隔符后没有元素，解析失败
                return Optional<std::pair<std::vector<ResultType>, string_view>>();
            }
            results.push_back(elem_result->first);
            current_input = elem_result->second;
        }
        
        return Optional<std::pair<std::vector<ResultType>, string_view>>(std::make_pair(results, current_input));
    };
}

// 组合器：delimited - 匹配由开始和结束解析器包围的内容
template <typename OpenParser, typename ContentParser, typename CloseParser>
Parser<typename ContentParser::result_type::value_type::first_type> 
delimited(OpenParser&& open, ContentParser&& content, CloseParser&& close) {
    using ResultType = typename ContentParser::result_type::value_type::first_type;
    
    return PARSER_LAMBDA {
        auto open_result = open(input);
        if (!open_result) {
            return Optional<std::pair<ResultType, string_view>>();
        }
        
        auto content_result = content(open_result->second);
        if (!content_result) {
            return Optional<std::pair<ResultType, string_view>>();
        }
        
        auto close_result = close(content_result->second);
        if (!close_result) {
            return Optional<std::pair<ResultType, string_view>>();
        }
        
        return Optional<std::pair<ResultType, string_view>>(std::make_pair(content_result->first, close_result->second));
    };
}

// 组合器：preceded - 匹配由前置解析器和内容解析器组成的结构，返回内容

template <typename PreParser, typename ContentParser>
Parser<typename ContentParser::result_type::value_type::first_type> 
preceded(PreParser&& pre, ContentParser&& content) {
    using ResultType = typename ContentParser::result_type::value_type::first_type;
    
    return PARSER_LAMBDA {
        auto pre_result = pre(input);
        if (!pre_result) {
            return Optional<std::pair<ResultType, string_view>>();
        }
        
        auto content_result = content(pre_result->second);
        if (!content_result) {
            return Optional<std::pair<ResultType, string_view>>();
        }
        
        return Optional<std::pair<ResultType, string_view>>(std::make_pair(content_result->first, content_result->second));
    };
}

// 组合器：terminated - 匹配由内容解析器和后置解析器组成的结构，返回内容
template <typename ContentParser, typename PostParser>
Parser<typename ContentParser::result_type::value_type::first_type> 
terminated(ContentParser&& content, PostParser&& post) {
    using ResultType = typename ContentParser::result_type::value_type::first_type;
    
    return PARSER_LAMBDA {
        auto content_result = content(input);
        if (!content_result) {
            return Optional<std::pair<ResultType, string_view>>();
        }
        
        auto post_result = post(content_result->second);
        if (!post_result) {
            return Optional<std::pair<ResultType, string_view>>();
        }
        
        return Optional<std::pair<ResultType, string_view>>(std::make_pair(content_result->first, post_result->second));
    };
}

// 组合器：map - 转换解析器结果
template <typename Parser, typename Transform>
Parser<decltype(std::declval<Transform>()(std::declval<typename Parser::result_type::value_type::first_type>()))> 
map(Parser&& parser, Transform&& transform) {
    using InputType = typename Parser::result_type::value_type::first_type;
    using OutputType = decltype(transform(std::declval<InputType>()));
    
    return PARSER_LAMBDA {
        auto result = parser(input);
        if (result) {
            return Optional<std::pair<OutputType, string_view>>(std::make_pair(
                transform(result->first),
                result->second
            ));
        }
        return Optional<std::pair<OutputType, string_view>>();
    };
}

// 组合器：flat_map - 扁平转换解析器结果
template <typename Parser, typename Transform>
Parser<typename std::result_of<Transform(typename Parser::result_type::value_type::first_type)>::type::result_type::value_type::first_type> 
flat_map(Parser&& parser, Transform&& transform) {
    using InputType = typename Parser::result_type::value_type::first_type;
    using OutputParser = typename std::result_of<Transform(InputType)>::type;
    using OutputType = typename OutputParser::result_type::value_type::first_type;
    
    return [parser = std::forward<Parser>(parser), transform = std::forward<Transform>(transform)](string_view input) -> Optional<std::pair<OutputType, string_view>> {
        auto result = parser(input);
        if (result) {
            auto output_parser = transform(result->first);
            return output_parser(result->second);
        }
        return Optional<std::pair<OutputType, string_view>>();
    };
}

#endif // COMBINATORS_H