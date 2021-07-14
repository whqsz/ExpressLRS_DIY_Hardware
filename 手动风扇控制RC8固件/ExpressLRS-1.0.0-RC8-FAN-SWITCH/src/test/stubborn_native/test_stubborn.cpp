#include <cstdint>
#include <telemetry_protocol.h>
#include <stubborn_sender.h>
#include <stubborn_receiver.h>
#include <unity.h>
#include <iostream>
#include <bitset>

StubbornSender sender(ELRS_TELEMETRY_MAX_PACKAGES);
StubbornReceiver receiver(ELRS_TELEMETRY_MAX_PACKAGES);

void test_stubborn_link_sends_data(void)
{
    uint8_t batterySequence[] = {0xEC,10, 0x08,0,0,0,0,0,0,0,0,109};
    sender.ResetState();
    sender.SetDataToTransmit(sizeof(batterySequence), batterySequence, 1);
    uint8_t *data;
    uint8_t maxLength;
    uint8_t packageIndex;
    bool confirmValue = true;

    for(int i = 0; i < sizeof(batterySequence); i++)
    {
        sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
        TEST_ASSERT_EQUAL(1, maxLength);
        TEST_ASSERT_EQUAL(i + 1, packageIndex);
        TEST_ASSERT_EQUAL(batterySequence[i], *data);
        sender.ConfirmCurrentPayload(confirmValue);
        confirmValue = !confirmValue;
    }

    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    TEST_ASSERT_EQUAL(0, packageIndex);
    TEST_ASSERT_EQUAL(true, sender.IsActive());
    sender.ConfirmCurrentPayload(!confirmValue);
    TEST_ASSERT_EQUAL(true, sender.IsActive());
    sender.ConfirmCurrentPayload(confirmValue);
    TEST_ASSERT_EQUAL(false, sender.IsActive());
}

void test_stubborn_link_sends_data_even_bytes_per_call(void)
{
    uint8_t batterySequence[] = {0xEC,10, 0x08,0,0,0,0,0,0,0,0,109};
    sender.ResetState();
    sender.SetDataToTransmit(sizeof(batterySequence), batterySequence, 2);
    uint8_t *data;
    uint8_t maxLength;
    uint8_t packageIndex;
    bool confirmValue = true;

    for(int i = 0; i < sizeof(batterySequence) / 2; i++)
    {
        sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
        TEST_ASSERT_EQUAL(2, maxLength);
        TEST_ASSERT_EQUAL(batterySequence[i*2], data[0]);
        TEST_ASSERT_EQUAL(batterySequence[i*2+1], data[1]);
        sender.ConfirmCurrentPayload(confirmValue);
        confirmValue = !confirmValue;
    }
    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    TEST_ASSERT_EQUAL(0, maxLength);
}

void test_stubborn_link_sends_data_odd_bytes_per_call(void)
{
    uint8_t batterySequence[] = {0xEC,11, 0x08,0,0,0,0,0,0,0,109};
    sender.ResetState();
    sender.SetDataToTransmit(sizeof(batterySequence), batterySequence, 3);
    uint8_t *data;
    uint8_t maxLength;
    uint8_t packageIndex;
    bool confirmValue = true;

    for(int i = 0; i < sizeof(batterySequence) / 3; i++)
    {
        sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
        TEST_ASSERT_EQUAL(3, maxLength);
        TEST_ASSERT_EQUAL(batterySequence[i*3], data[0]);
        TEST_ASSERT_EQUAL(batterySequence[i*3+1], data[1]);
        TEST_ASSERT_EQUAL(batterySequence[i*3+2], data[2]);
        sender.ConfirmCurrentPayload(confirmValue);
        confirmValue = !confirmValue;
    }
    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    TEST_ASSERT_EQUAL(2, maxLength);
    TEST_ASSERT_EQUAL(batterySequence[9], data[0]);
    TEST_ASSERT_EQUAL(batterySequence[10], data[1]);

    sender.ConfirmCurrentPayload(confirmValue);
    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    TEST_ASSERT_EQUAL(0, maxLength);
}

void test_stubborn_link_sends_data_larger_frame_size(void)
{
    uint8_t batterySequence[] = {0xEC,10, 0x08,0,0,0,0,0,0,0,0,109};
    sender.ResetState();
    sender.SetDataToTransmit(sizeof(batterySequence), batterySequence, 13);
    uint8_t *data;
    uint8_t maxLength;
    uint8_t packageIndex;
    bool confirmValue = true;

    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    TEST_ASSERT_EQUAL(12, maxLength);
    for(int i = 0; i < sizeof(batterySequence); i++)
    {
        TEST_ASSERT_EQUAL(batterySequence[i], data[i]);
    }

    sender.ConfirmCurrentPayload(confirmValue);
    confirmValue = !confirmValue;
    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    TEST_ASSERT_EQUAL(0, maxLength);
}

void test_stubborn_link_receives_data(void)
{
    volatile uint8_t batterySequence[] = {0xEC,10, 0x08,0,0,0,0,0,0,0,0,109};
    uint8_t buffer[100];
    receiver.ResetState();
    receiver.SetDataToReceive(sizeof(buffer), buffer, 1);

    for(int i = 0; i < sizeof(batterySequence); i++)
    {
        receiver.ReceiveData(i+1, batterySequence + i);
    }
    receiver.ReceiveData(0, 0);

    for(int i = 0; i < sizeof(batterySequence); i++)
    {
        TEST_ASSERT_EQUAL(batterySequence[i], buffer[i]);
    }

    receiver.Unlock();
}

void test_stubborn_link_receives_data_with_multiple_bytes(void)
{
    volatile uint8_t batterySequence[] = {0xEC,10, 0x08,0,0,0,0,0,0,0,0,109,0,0,0};
    uint8_t buffer[100];
    receiver.ResetState();
    receiver.SetDataToReceive(sizeof(buffer), buffer, 5);

    for(int i = 0; i < sizeof(batterySequence) / 3; i++)
    {
        receiver.ReceiveData(i+1, batterySequence + (i * 5));
    }
    receiver.ReceiveData(0, 0);

    for(int i = 0; i < sizeof(batterySequence); i++)
    {
        TEST_ASSERT_EQUAL(batterySequence[i], buffer[i]);
    }

    receiver.Unlock();
}

void test_stubborn_link_resyncs(void)
{
    uint8_t batterySequence[] = {0xEC,10, 0x08,0,0,0,0,0,0,0,0,109};
    uint8_t buffer[100];
    uint8_t *data;
    uint8_t maxLength;
    uint8_t packageIndex;

    receiver.ResetState();
    receiver.SetDataToReceive(sizeof(buffer), buffer, 1);

    sender.ResetState();
    sender.SetDataToTransmit(sizeof(batterySequence), batterySequence, 1);

    // send and confirm two packages
    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    receiver.ReceiveData(packageIndex, data);
    sender.ConfirmCurrentPayload(receiver.GetCurrentConfirm());

    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    receiver.ReceiveData(packageIndex, data);
    sender.ConfirmCurrentPayload(receiver.GetCurrentConfirm());

    // now reset receiver so the receiver sends the wrong confirm value
    // all communication would be stuck
    receiver.ResetState();
    receiver.SetDataToReceive(sizeof(buffer), buffer, 1);

    // wait for resync to happen
    for(int i = 0; i < sender.GetMaxPacketsBeforeResync() + 1; i++)
    {
        sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
        TEST_ASSERT_EQUAL(3, packageIndex);
        receiver.ReceiveData(packageIndex, data);
        sender.ConfirmCurrentPayload(receiver.GetCurrentConfirm());
    }

    // resync active
    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    TEST_ASSERT_EQUAL(ELRS_TELEMETRY_MAX_PACKAGES, packageIndex);
    receiver.ReceiveData(packageIndex, data);
    sender.ConfirmCurrentPayload(receiver.GetCurrentConfirm());
    TEST_ASSERT_EQUAL(false, sender.IsActive());

    // both are in sync again
    sender.SetDataToTransmit(sizeof(batterySequence), batterySequence, 1);
    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    receiver.ReceiveData(packageIndex, data);
    sender.ConfirmCurrentPayload(receiver.GetCurrentConfirm());

    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    TEST_ASSERT_EQUAL(2, packageIndex);

    receiver.Unlock();
}

void test_stubborn_link_resyncs_during_last_confirm(void)
{
    uint8_t batterySequence[] = {0xEC,10, 0x08,0,0,0,0,0,0,0,0,109};
    uint8_t buffer[100];
    uint8_t *data;
    uint8_t maxLength;
    uint8_t packageIndex;

    receiver.ResetState();
    receiver.SetDataToReceive(sizeof(buffer), buffer, 1);

    sender.ResetState();
    sender.SetDataToTransmit(sizeof(batterySequence), batterySequence, 1);

    // send and confirm twelfe packages
    for (int i = 0; i < sizeof(batterySequence); i++)
    {
        sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
        receiver.ReceiveData(packageIndex, data);
        sender.ConfirmCurrentPayload(receiver.GetCurrentConfirm());
    }

    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    TEST_ASSERT_EQUAL(0, packageIndex);

    // now reset receiver so the receiver sends the wrong confirm value
    // all communication would be stuck since sender waits for final confirmation
    receiver.ResetState();
    receiver.SetDataToReceive(sizeof(buffer), buffer, 1);

    // wait for resync to happen
    for(int i = 0; i < sender.GetMaxPacketsBeforeResync() + 1; i++)
    {
        sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
        TEST_ASSERT_EQUAL(0, packageIndex);
        receiver.ReceiveData(packageIndex, data);
        sender.ConfirmCurrentPayload(receiver.GetCurrentConfirm());
    }

    // resync active
    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    TEST_ASSERT_EQUAL(ELRS_TELEMETRY_MAX_PACKAGES, packageIndex);
    receiver.ReceiveData(packageIndex, data);
    sender.ConfirmCurrentPayload(receiver.GetCurrentConfirm());
    TEST_ASSERT_EQUAL(false, sender.IsActive());

    // both are in sync again
    sender.SetDataToTransmit(sizeof(batterySequence), batterySequence, 1);
    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    receiver.ReceiveData(packageIndex, data);
    sender.ConfirmCurrentPayload(receiver.GetCurrentConfirm());

    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    TEST_ASSERT_EQUAL(2, packageIndex);

    receiver.Unlock();
}

void test_stubborn_link_sends_data_until_confirmation(void)
{
    uint8_t batterySequence[] = {0xEC,10, 0x08,0,0,0,0,0,0,0,0,109};
    uint8_t *data;
    uint8_t maxLength;
    uint8_t packageIndex;
    uint8_t buffer[100];

    sender.ResetState();
    sender.SetDataToTransmit(sizeof(batterySequence), batterySequence, 1);

    receiver.ResetState();
    receiver.SetDataToReceive(sizeof(buffer), buffer, 1);

    for(int i = 0; i < 100; i++)
    {
        sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
        TEST_ASSERT_EQUAL(1, packageIndex);
        receiver.ReceiveData(packageIndex, data);
        sender.ConfirmCurrentPayload(!receiver.GetCurrentConfirm());
    }

    sender.ConfirmCurrentPayload(receiver.GetCurrentConfirm());
    sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
    TEST_ASSERT_EQUAL(2, packageIndex);
}

void test_stubborn_link_multiple_packages(void)
{
    uint8_t batterySequence[] = {0xEC,10, 0x08,0,0,0,0,0,0,0,0,109};
    uint8_t *data;
    uint8_t maxLength;
    uint8_t packageIndex;
    uint8_t buffer[100];

    receiver.ResetState();
    receiver.SetDataToReceive(sizeof(buffer), buffer, 1);
    sender.ResetState();

    for (int i = 0; i < 3; i++)
    {
        sender.SetDataToTransmit(sizeof(batterySequence), batterySequence, 1);
        TEST_ASSERT_EQUAL(true, sender.IsActive());
        for(int currentByte = 0; currentByte <= sizeof(batterySequence); currentByte++)
        {
            sender.GetCurrentPayload(&packageIndex, &maxLength, &data);
            receiver.ReceiveData(packageIndex, data);
            sender.ConfirmCurrentPayload(receiver.GetCurrentConfirm());
        }

        TEST_ASSERT_EQUAL(false, sender.IsActive());
        TEST_ASSERT_EQUAL(true, receiver.HasFinishedData());
        receiver.Unlock();
        TEST_ASSERT_EQUAL(false, receiver.HasFinishedData());
    }

}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_stubborn_link_sends_data);
    RUN_TEST(test_stubborn_link_sends_data_even_bytes_per_call);
    RUN_TEST(test_stubborn_link_sends_data_odd_bytes_per_call);
    RUN_TEST(test_stubborn_link_sends_data_larger_frame_size);
    RUN_TEST(test_stubborn_link_receives_data);
    RUN_TEST(test_stubborn_link_receives_data_with_multiple_bytes);
    RUN_TEST(test_stubborn_link_resyncs);
    RUN_TEST(test_stubborn_link_sends_data_until_confirmation);
    RUN_TEST(test_stubborn_link_multiple_packages);
    RUN_TEST(test_stubborn_link_resyncs_during_last_confirm);
    UNITY_END();

    return 0;
}
