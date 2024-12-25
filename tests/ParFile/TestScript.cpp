// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Script.h>

#include <TestParFile/test.h>

#include <ParFile/Config.h>
#include <ParFile/Json.h>

#include <gtest/gtest.h>

TEST(TestScript, construct)
{
    boost::json::object m_json{ParFile::read_json(TestParFile::CONFIG_JSON).as_object()};
    ParFile::Config config{ParFile::Config{m_json}};

    ParFile::Script script{config};
}

TEST(TestScript, commandForFrame)
{
    boost::json::object m_json{ParFile::read_json(TestParFile::CONFIG_JSON).as_object()};
    ParFile::Config config{ParFile::Config{m_json}};
    ParFile::Script script{config};

    const std::string commands{script.commands("frame-0001")};

    EXPECT_EQ(std::string{"start/wait id @"} + TestParFile::TEST_OUTPUT +
            "/frame-0001\n"
            "if errorlevel 1 exit /b 1\n",
        commands);
}
