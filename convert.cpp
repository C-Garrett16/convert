//#include <cstddef>
#include <exception>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <cctype>


std::unordered_map<std::string, double> length_factors {
    {"m", 1.0},
    {"cm", 0.01},
    {"mm", 0.001},
    {"ft", 0.3048},
    {"yd", 0.9144},
    {"km", 1000},
    {"mi", 1609.34}
};

std::unordered_map<std::string, double> mass_factors {
    {"kg", 1.0},
    {"g", 0.001},
    {"lb", 0.453592},
    {"oz", 0.0283495}
};

std::unordered_map<std::string, double> volume_factors {
    {"L",      1.0},          // liter
    {"l",      1.0},          // lowercase alias
    {"mL",     0.001},        // milliliter
    {"ml",     0.001},        // lowercase alias
    {"uL",     0.000001},     // microliter
    {"ul",     0.000001},     // lowercase alias

    {"gal",    3.78541},      // US gallon
    {"qt",     0.946353},     // US quart
    {"pt",     0.473176},     // US pint
    {"cup",    0.24},         // metric cup
    {"floz",   0.0295735},    // US fluid ounce

    {"tbsp",   0.0147868},    // tablespoon
    {"tsp",    0.00492892},   // teaspoon

    {"m3",     1000.0},       // cubic meter
    {"cm3",    0.001},        // cubic centimeter = milliliter
    {"cc",     0.001},        // cc (same as mL)
    {"in3",    0.0163871},    // cubic inch
    {"ft3",    28.3168},      // cubic foot
};

struct TempUnit {
    std::function<double(double)> to_celsius;
    std::function<double(double)> from_celsius;
};

std::unordered_map<std::string, TempUnit> temp_units {
    // Celsius
    {"C", TempUnit {
        [](double c) { return c; },
        [](double c) { return c; }
    }},
    // Fahrenheit
    {"F", TempUnit {
        [](double f) { return (f - 32.0) * 5.0 / 9.0; },
        [](double c) { return c * 9.0 / 5.0 + 32.0; }
    }},
    // Kelvin
    {"K", TempUnit {
        [](double k) { return k - 273.15; },
        [](double c) { return c + 273.15; }
    }},
};

// std::unordered_map<std::string, double> currency_factors {   <-- This is a WIP
//     { "USD", 1.0 },
//     { "EUR", 1.1 },
//     { "GBP", 1.3 },
//     { "AUD", 1.4 },
//     { "CNY", 0.3 },
//     { "JPY", 0.8 }
// };

// Maps "weird user input" -> canonical unit key used in your factor maps
std::unordered_map<std::string, std::string> unit_aliases = {
    // length
    {"meter", "m"}, {"meters", "m"},
    {"metre", "m"}, {"metres", "m"},
    {"kilometer", "km"}, {"kilometers", "km"},
    {"kilometre", "km"}, {"kilometres", "km"},
    {"foot", "ft"}, {"feet", "ft"},
    {"yard", "yd"}, {"yards", "yd"},
    {"mile", "mi"}, {"miles", "mi"},

    // mass
    {"kilogram", "kg"}, {"kilograms", "kg"},
    {"gram", "g"}, {"grams", "g"},
    {"pound", "lb"}, {"pounds", "lb"},
    {"lbs", "lb"},    // common typo / plural
    {"ounce", "oz"}, {"ounces", "oz"},

    // volume
    {"liter", "L"}, {"liters", "L"},
    {"litre", "L"}, {"litres", "L"},
    {"milliliter", "mL"}, {"milliliters", "mL"},
    {"millilitre", "mL"}, {"millilitres", "mL"},
    {"cup", "cup"}, {"cups", "cup"},
    {"tablespoon", "tbsp"}, {"tablespoons", "tbsp"},
    {"teaspoon", "tsp"}, {"teaspoons", "tsp"},

    // temperature
    {"c", "C"}, {"celsius", "C"}, {"centigrade", "C"},
    {"f", "F"}, {"fahrenheit", "F"},
    {"k", "K"}, {"kelvin", "K"},
};


void print_usage() {
    std::cout << "Usage: convert -f (From Unit) -t (To Unit) <num>" << std::endl; 
}

struct ConversionRule {
    std::string from_unit;
    std::string to_unit;
    std::function<double(double)> convert;
};



struct Args {
    std::string from_unit;
    std::string to_unit;
    double value;
    bool show_help {false};
    bool list_units {false};
};

std::string to_lower(std::string s) {
    for (char &ch : s) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return s;
};

std::string normalize_unit(std::string u) {

    std::string lower = to_lower(u);

    auto it = unit_aliases.find(lower);
    if (it != unit_aliases.end()) {
        return it->second;
    }

    return u;
}

Args parse_args(int argc, char* argv[]) {
    // Argument handling
    Args result;
    bool have_from {false};
    bool have_to {false};
    bool have_value {false};

    for (int i = 1; i < argc; ++i) {
        
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            result.show_help = true;
        } else if (arg == "-l" || arg == "--list" || arg == "--units") {
            result.list_units = true;
        } else if (arg == "-f" || arg == "--from") {
            if (i + 1 < argc) {
                if (i+1 >= argc) {
                    throw std::runtime_error("'-f/--from' flag requires a unit.");
                }
                result.from_unit = normalize_unit(argv[++i]);
                have_from = true;
            }
        } else if (arg == "-t" || arg == "--to") {
            if (i + 1 < argc) {
                if (i+1 >= argc) {
                    throw std::runtime_error("'-t/--to' flag requires a unit.");
                }
                result.to_unit = normalize_unit(argv[++i]);
                have_to = true;
            }
        } else {
            try {
                result.value = std::stod(arg);
                have_value = true;
        
            }
            catch (const std::exception&) {
                throw std::invalid_argument("Value must be a valid number.");
            }
        }
    }

    if (!result.list_units && !result.show_help) {
        if (!have_from || !have_to || !have_value) {
            throw std::runtime_error("Missing required arguments");
        }
    }

    return result;
}


double convert(std::string from_unit, std::string to_unit, double value) {

    bool from_is_length = length_factors.count(from_unit) > 0;
    bool to_is_length = length_factors.count(to_unit) > 0;

    bool from_is_mass = mass_factors.count(from_unit) > 0;
    bool to_is_mass = mass_factors.count(to_unit) > 0;

    bool from_is_volume = volume_factors.count(from_unit) > 0;
    bool to_is_volume = volume_factors.count(to_unit) > 0;

    bool from_is_temp = temp_units.count(from_unit) > 0;
    bool to_is_temp = temp_units.count(to_unit) > 0;

    if (length_factors.count(from_unit) > 0 &&
        length_factors.count(to_unit) > 0) {
            double from_factor = length_factors.at(from_unit);
            double to_factor = length_factors.at(to_unit);

            double value_in_meters = value * from_factor;
            double result = value_in_meters / to_factor;

            return result;
        }
    if (mass_factors.count(from_unit) > 0 &&
        mass_factors.count(to_unit) > 0) {
            double from_factor = mass_factors.at(from_unit);
            double to_factor = mass_factors.at(to_unit);

            double value_in_kilograms = value * from_factor;
            double result = value_in_kilograms / to_factor;

            return result;
        }

    if (volume_factors.count(from_unit) > 0 &&
        volume_factors.count(to_unit) > 0) {
            double from_factor = volume_factors.at(from_unit);
            double to_factor = volume_factors.at(to_unit);

            double value_in_liters = value * from_factor;
            double result = value_in_liters / to_factor;

            return result;
        }

    if (temp_units.count(from_unit) > 0 &&
        temp_units.count(to_unit) > 0) {
            double temp_in_celcius = temp_units[from_unit].to_celsius(value);
            double result = temp_units[to_unit].from_celsius(temp_in_celcius);

            return result;
        }
        
    // if (currency_factors.count(from_unit) > 0 &&         <-- WIP
    //     currency_factors.count(to_unit) > 0) {
    //         double from_factor = currency_factors.at(from_unit);
    //         double to_factor = currency_factors.at(to_unit);

    //         double value_in_dollars = value * from_factor;
    //         double result = value_in_dollars / to_factor;

    //         return result;
    //     }
    
    if ((from_is_length && !to_is_length) ||
        (from_is_mass && !to_is_mass)     ||
        (from_is_volume && !to_is_volume) ||
        (from_is_temp && !to_is_temp))  {
            throw std::runtime_error("Incompatible unit types (eg., length vs. mass)");
    }

    else {
        throw std::runtime_error("Unknown unit(s): from='" + from_unit + "', to='" + to_unit + "'");
    }
}

void print_units() // --> Start here. <--




int main(int argc, char* argv[]) {
    try {
        Args args = parse_args(argc, argv);

        if (args.from_unit == "" || args.to_unit == "") {
            print_usage();
            return 1;
        }

        double result = convert(args.from_unit, args.to_unit, args.value);
    
        // Print out values.
        std::cout << "From: " << args.from_unit << "\n"
                  << "To: " << args.to_unit << "\n"
                  << "Value: \033[1;32m" << result << args.to_unit << "\033[0m\n";

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "\033[1;31mError: \033[31m" << e.what() << "\033[0m\n";
        print_usage();
        return 1;
    }
}