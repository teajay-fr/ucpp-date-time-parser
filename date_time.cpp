// Copyright (C) 2014, Richard Thomson.  All rights reserved.
#include "date_time.h"
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <stdexcept>

using namespace boost::spirit::qi;

struct date_part
{
    date_time::days week_day;
    unsigned year;
    date_time::months month;
    unsigned day;
};

BOOST_FUSION_ADAPT_STRUCT(date_part,
    (date_time::days, week_day)
    (unsigned, day)
    (date_time::months, month)
    (unsigned, year)
    );

struct time_part
{
    unsigned hour;
    unsigned minute;
    unsigned second;
    int time_zone_offset; 
};

BOOST_FUSION_ADAPT_STRUCT(
    time_part,
    (unsigned, hour)
    (unsigned, minute)
    (unsigned, second)
    (int, time_zone_offset)
    );

BOOST_FUSION_ADAPT_STRUCT(date_time::moment,
    (date_time::days, week_day)
    (unsigned, day)
    (date_time::months, month)
    (unsigned, year)
    (unsigned, hour)
    (unsigned, minute)
    (unsigned, second)
    (int, time_zone_offset)
);

// we provide a custom attribute transformation to allow its use as an int
namespace boost { namespace spirit { namespace qi
{
    // in this case we just expose the embedded 'int' as the attribute instance
    // to use, allowing to leave the function 'post()' empty
    template <>
    struct transform_attribute<date_time::moment, boost::fusion::vector2<date_part, time_part> >
    {
        typedef boost::fusion::vector2<date_part, time_part> val_type;
        typedef val_type &type;
        static type pre(date_time::moment& d) { return reinterpret_cast<type>(d); }
        static void post(date_time::moment& val, val_type const& attr) {}
        static void fail(date_time::moment&) {}
    };
    
}}}

namespace
{

typedef ascii::space_type skipper;

template <typename Iter>
struct date_time_grammar : grammar<Iter, date_time::moment(), skipper>
{
    date_time_grammar() : date_time_grammar::base_type{start}
    {
        month_names.add("Jan", date_time::January)
            ("Feb", date_time::February)
            ("Mar", date_time::March)
            ("Apr", date_time::April)
            ("May", date_time::May)
            ("Jun", date_time::June)
            ("Jul", date_time::July)
            ("Aug", date_time::August)
            ("Sep", date_time::September)
            ("Oct", date_time::October)
            ("Nov", date_time::November)
            ("Dec", date_time::December);
        day_names.add("Mon", date_time::Monday)
            ("Tue", date_time::Tuesday)
            ("Wed", date_time::Wednesday)
            ("Thu", date_time::Thursday)
            ("Fri", date_time::Friday)
            ("Sat", date_time::Saturday)
            ("Sun", date_time::Sunday);
        uint_parser<unsigned, 10, 1, 2> digit_1_2;
        uint_parser<unsigned, 10, 2, 2> digit_2;
        uint_parser<unsigned, 10, 4, 4> digit_4;
        int_parser<int, 10, 4, 4> time_zone_offset;
        seconds = no_skip[(':' >> digit_2) | attr(0)];
        week_day = (day_names >> ',') | attr(date_time::Unspecified);
        date = week_day >> digit_1_2 >> month_names >> digit_4;
        time = digit_2 >> no_skip[lit(':')] >> no_skip[digit_2] >> seconds >> time_zone_offset;
        start = attr_cast(date >> time);
    };

    symbols<char const, date_time::days> day_names;
    rule<Iter, date_time::days()> week_day;
    rule<Iter, unsigned(), skipper> seconds;
    rule<Iter, date_part(), skipper> date;
    rule<Iter, time_part(), skipper> time;
    symbols<char const, date_time::months> month_names;
    rule<Iter, date_time::moment(), skipper> start;
};

}

namespace date_time
{

moment parse(std::string const& text)
{
    moment result{};
    std::string::const_iterator start{text.begin()};
    if (phrase_parse(start, text.end(),
            date_time_grammar<std::string::const_iterator>{},
            ascii::space, result)
        && start == text.end())
    {
        return result;
    }

    throw std::domain_error("invalid date time");
}

}
