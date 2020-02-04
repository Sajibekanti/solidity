/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "libyul/AsmAnalysis.h"
#include "libyul/AsmAnalysisInfo.h"
#include <libyul/AsmParser.h>

#include "libyul/backends/wasm/WasmDialect.h"
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/wasm/WasmDialect.h>

#include "liblangutil/EVMVersion.h"
#include <liblangutil/Exceptions.h>

#include <test/libyul/SyntaxTest.h>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::yul::test;

namespace
{
std::map<string const, yul::Dialect const& (*)(langutil::EVMVersion)> validDialects = {
	{
		"evm",
		[](langutil::EVMVersion _evmVersion) -> yul::Dialect const&
		{ return yul::EVMDialect::strictAssemblyForEVM(_evmVersion); }
	},
	{
		"evmTyped",
		[](langutil::EVMVersion _evmVersion) -> yul::Dialect const&
		{ return yul::EVMDialectTyped::strictAssemblyForEVM(_evmVersion); }
	},
	{
		"yul",
		[](langutil::EVMVersion) -> yul::Dialect const&
		{ return yul::Dialect::yulDeprecated(); }
	},
	{
		"ewasm",
		[](langutil::EVMVersion) -> yul::Dialect const&
		{ return yul::WasmDialect::instance(); }
	}
};

vector<string> const validDialectNames()
{
	vector<string> names{size(validDialects), ""};
	transform(begin(validDialects), end(validDialects), names.begin(), [](auto const& dialect) { return dialect.first; });

	return names;
}
}

void SyntaxTest::parseAndAnalyze()
{
	string dialectName = m_validatedSettings.count("Dialect") ? m_validatedSettings["Dialect"] : "evmTyped";

	yul::Dialect const& dialect = validDialects[dialectName](m_evmVersion);

	for (auto const& pair: m_sources)
		try
		{
			ErrorList errorList{};
			ErrorReporter errorReporter{errorList};

			auto scanner = make_shared<Scanner>(CharStream(pair.second, pair.first));
			auto parserResult = yul::Parser(errorReporter, dialect).parse(scanner, false);

			if (parserResult)
			{
				yul::AsmAnalysisInfo analysisInfo;
				yul::AsmAnalyzer(analysisInfo, errorReporter, dialect).analyze(*parserResult);
			}

			for (auto const& error: errorList)
			{
				int locationStart = -1, locationEnd = -1;

				if (auto location = boost::get_error_info<errinfo_sourceLocation>(*error))
				{
					locationStart = location->start;
					locationEnd = location->end;
				}

				m_errorList.emplace_back(SyntaxTestError{
					error->typeName(),
					errorMessage(*error),
					pair.first,
					locationStart,
					locationEnd
				});
			}
		}
		catch (UnimplementedFeatureError const& _e)
		{
			m_errorList.emplace_back(SyntaxTestError{
				"UnimplementedFeatureError",
				errorMessage(_e),
				"",
				-1,
				-1
			});
		}
}

bool SyntaxTest::validateSettings(langutil::EVMVersion _evmVersion)
{
	if (!CommonSyntaxTest::validateSettings(_evmVersion))
		return false;

	if (!m_settings.count("Dialect"))
		return true;

	string const dialect = m_settings["Dialect"];
	m_validatedSettings["Dialect"] = dialect;
	m_settings.erase("Dialect");

	if (!validDialects.count(dialect))
		BOOST_THROW_EXCEPTION(runtime_error{
			"Invalid Dialect \"" +
			dialect +
			"\". Valid dialects are " +
			joinHumanReadable(validDialectNames(), ", ", " and ") +
			"."
		});

	return true;
}
