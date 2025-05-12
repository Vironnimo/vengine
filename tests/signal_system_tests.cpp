#include <doctest.h>
#include "vengine/core/signal_system.hpp"

TEST_CASE("SubscribeAndEmit") {
    Vengine::SignalSystem signalSystem;
    bool eventTriggered = false;
    signalSystem.subscribe("test_signal", [&eventTriggered](void* data) {
        eventTriggered = true;
    });

    signalSystem.publish("test_signal");
    CHECK(eventTriggered);
}

TEST_CASE("EmitWithData") {
    Vengine::SignalSystem signalSystem;
    int eventData = 0;
    signalSystem.subscribe("test_signal", [&eventData](void* data) {
        eventData = *static_cast<int*>(data);
    });

    int data = 42;
    signalSystem.publish("test_signal", &data);
    CHECK(eventData == 42);  // should change
}

TEST_CASE("MultipleSubscribers") {
    Vengine::SignalSystem signalSystem;
    float eventData1 = 0.0f;
    int eventData2 = 0;

    signalSystem.subscribe("test_signal", [&eventData1](void* data) {
        eventData1 = *static_cast<float*>(data);
    });

    signalSystem.subscribe("test_signal", [&eventData2](void* data) {
        eventData2 = *static_cast<int*>(data);
    });

    int data = 42;
    signalSystem.publish("test_signal", &data);
    CHECK(eventData1 != 42.0f);
    CHECK(eventData2 == 42);
}

TEST_CASE("NoSubscribers") {
    Vengine::SignalSystem signalSystem;
    // publish event with no subscribers, should not cause any issues
    CHECK_NOTHROW(signalSystem.publish("non_existent_event"));
}

TEST_CASE("EmitWithoutSubscription") {
    Vengine::SignalSystem signalSystem;
    bool eventTriggered = false;
    signalSystem.publish("test_signal");
    CHECK(eventTriggered == false);
}

TEST_CASE("EmitWithIncorrectData") {
    Vengine::SignalSystem signalSystem;
    int eventData = 0;
    signalSystem.subscribe("test_signal", [&eventData](void* data) {
        eventData = *static_cast<int*>(data);
    });

    float incorrectData = 42.0f;
    signalSystem.publish("test_signal", &incorrectData);
    CHECK(eventData != 42);  // incorrect type, shoudn't change
}

TEST_CASE("EmitWithNullData") {
    Vengine::SignalSystem signalSystem;
    int eventData = 0;
    signalSystem.subscribe("test_signal", [&eventData](void* data) {
        if (data) {
            eventData = *static_cast<int*>(data);
        }
    });

    signalSystem.publish("test_signal", nullptr);
    CHECK(eventData == 0); // no data, no change
}

TEST_CASE("MultipleSubscribersWithDifferentData") {
    Vengine::SignalSystem signalSystem;
    int eventData1 = 0;
    float eventData2 = 0.0f;

    signalSystem.subscribe("test_signal", [&eventData1](void* data) {
        eventData1 = *static_cast<int*>(data);
    });

    signalSystem.subscribe("test_signal", [&eventData2](void* data) {
        eventData2 = *static_cast<float*>(data);
    });

    int data = 42;
    signalSystem.publish("test_signal", &data);
    CHECK(eventData1 == 42);  // should change
    CHECK(eventData2 != 42.0f);  // shouldn't change because wrong datatype
}