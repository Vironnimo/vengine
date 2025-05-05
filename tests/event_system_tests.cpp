#include <doctest.h>
#include "vengine/core/signal_system.hpp"

TEST_CASE("SubscribeAndEmit") {
    Vengine::SignalSystem eventSystem;
    bool eventTriggered = false;
    eventSystem.subscribe("test_event", [&eventTriggered](void* data) {
        eventTriggered = true;
    });

    eventSystem.publish("test_event");
    CHECK(eventTriggered);
}

TEST_CASE("EmitWithData") {
    Vengine::SignalSystem eventSystem;
    int eventData = 0;
    eventSystem.subscribe("test_event", [&eventData](void* data) {
        eventData = *static_cast<int*>(data);
    });

    int data = 42;
    eventSystem.publish("test_event", &data);
    CHECK(eventData == 42);  // should change
}

TEST_CASE("MultipleSubscribers") {
    Vengine::SignalSystem eventSystem;
    float eventData1 = 0.0f;
    int eventData2 = 0;

    eventSystem.subscribe("test_event", [&eventData1](void* data) {
        eventData1 = *static_cast<float*>(data);
    });

    eventSystem.subscribe("test_event", [&eventData2](void* data) {
        eventData2 = *static_cast<int*>(data);
    });

    int data = 42;
    eventSystem.publish("test_event", &data);
    CHECK(eventData1 != 42.0f);  
    CHECK(eventData2 == 42);  
}

TEST_CASE("NoSubscribers") {
    Vengine::SignalSystem eventSystem;
    // publish event with no subscribers, should not cause any issues
    CHECK_NOTHROW(eventSystem.publish("non_existent_event"));
}

TEST_CASE("EmitWithoutSubscription") {
    Vengine::SignalSystem eventSystem;
    bool eventTriggered = false;
    eventSystem.publish("test_event");
    CHECK(eventTriggered == false);  
}

TEST_CASE("EmitWithIncorrectData") {
    Vengine::SignalSystem eventSystem;
    int eventData = 0;
    eventSystem.subscribe("test_event", [&eventData](void* data) {
        eventData = *static_cast<int*>(data);
    });

    float incorrectData = 42.0f;
    eventSystem.publish("test_event", &incorrectData);
    CHECK(eventData != 42);  // incorrect type, shoudn't change
}

TEST_CASE("EmitWithNullData") {
    Vengine::SignalSystem eventSystem;
    int eventData = 0;
    eventSystem.subscribe("test_event", [&eventData](void* data) {
        if (data) {
            eventData = *static_cast<int*>(data);
        }
    });

    eventSystem.publish("test_event", nullptr);
    CHECK(eventData == 0); // no data, no change
}

TEST_CASE("MultipleSubscribersWithDifferentData") {
    Vengine::SignalSystem eventSystem;
    int eventData1 = 0;
    float eventData2 = 0.0f;

    eventSystem.subscribe("test_event", [&eventData1](void* data) {
        eventData1 = *static_cast<int*>(data);
    });

    eventSystem.subscribe("test_event", [&eventData2](void* data) {
        eventData2 = *static_cast<float*>(data);
    });

    int data = 42;
    eventSystem.publish("test_event", &data);
    CHECK(eventData1 == 42);  // should change
    CHECK(eventData2 != 42.0f);  // shoudn't change because wrong datatype
}