#include <unity.h>
#include <math.h>

#include "Modules/PoolLogicModule/FiltrationWindow.h"

void test_temp_below_low_uses_min_duration()
{
    FiltrationWindowInput in{};
    in.waterTemp = 5.0f;
    in.lowThreshold = 12.0f;
    in.setpoint = 24.0f;
    in.startMinHour = 8;
    in.stopMaxHour = 23;

    FiltrationWindowOutput out{};
    TEST_ASSERT_TRUE(computeFiltrationWindowDeterministic(in, out));
    TEST_ASSERT_EQUAL_UINT8(2, out.durationHours);
    TEST_ASSERT_EQUAL_UINT8(14, out.startHour);
    TEST_ASSERT_EQUAL_UINT8(16, out.stopHour);
}

void test_temp_equal_setpoint_switches_to_high_factor()
{
    FiltrationWindowInput in{};
    in.waterTemp = 24.0f;
    in.lowThreshold = 12.0f;
    in.setpoint = 24.0f;
    in.startMinHour = 8;
    in.stopMaxHour = 23;

    FiltrationWindowOutput out{};
    TEST_ASSERT_TRUE(computeFiltrationWindowDeterministic(in, out));
    // 24 * 0.5 = 12h
    TEST_ASSERT_EQUAL_UINT8(12, out.durationHours);
    TEST_ASSERT_EQUAL_UINT8(9, out.startHour);
    TEST_ASSERT_EQUAL_UINT8(21, out.stopHour);
}

void test_nan_temperature_returns_false()
{
    FiltrationWindowInput in{};
    in.waterTemp = NAN;
    in.lowThreshold = 12.0f;
    in.setpoint = 24.0f;
    in.startMinHour = 8;
    in.stopMaxHour = 23;

    FiltrationWindowOutput out{};
    TEST_ASSERT_FALSE(computeFiltrationWindowDeterministic(in, out));
}

void test_stop_le_start_uses_emergency_duration_when_possible()
{
    FiltrationWindowInput in{};
    in.waterTemp = 20.0f; // duration about 7h
    in.lowThreshold = 12.0f;
    in.setpoint = 24.0f;
    in.startMinHour = 22;
    in.stopMaxHour = 22; // force stop clamp to start

    FiltrationWindowOutput out{};
    TEST_ASSERT_TRUE(computeFiltrationWindowDeterministic(in, out));
    TEST_ASSERT_EQUAL_UINT8(22, out.startHour);
    TEST_ASSERT_EQUAL_UINT8(23, out.stopHour);
    TEST_ASSERT_EQUAL_UINT8(1, out.durationHours);
}

void test_stop_le_start_with_late_start_uses_fallback_window()
{
    FiltrationWindowInput in{};
    in.waterTemp = 20.0f;
    in.lowThreshold = 12.0f;
    in.setpoint = 24.0f;
    in.startMinHour = 23;
    in.stopMaxHour = 23; // start gets pinned to 23

    FiltrationWindowOutput out{};
    TEST_ASSERT_TRUE(computeFiltrationWindowDeterministic(in, out));
    TEST_ASSERT_EQUAL_UINT8(22, out.startHour);
    TEST_ASSERT_EQUAL_UINT8(23, out.stopHour);
    TEST_ASSERT_EQUAL_UINT8(1, out.durationHours);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_temp_below_low_uses_min_duration);
    RUN_TEST(test_temp_equal_setpoint_switches_to_high_factor);
    RUN_TEST(test_nan_temperature_returns_false);
    RUN_TEST(test_stop_le_start_uses_emergency_duration_when_possible);
    RUN_TEST(test_stop_le_start_with_late_start_uses_fallback_window);
    return UNITY_END();
}
