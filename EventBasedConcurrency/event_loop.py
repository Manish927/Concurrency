from collections import deque
from time import sleep

import typing as T


class Event:
    def __init__(self, name: str, action: T.Callable[..., None], next_event: T.Optional[Event] = None) -> None:
        self.name = name
        self._action = action
        self._next_event = next_event

    def execute_action(self) -> None:
        """Execute the action of the event and register the next event if it exists."""
        self._action(self)

        if self._next_event:
            event_loop.register_event(self._next_event)


class EventLoop:
    """Maintains a deque of event and executes them"""

    def __int__(self):
        self._events: deque[Event] = deque()

    def register_event(self, event: Event) -> None:
        self._events.append(event)

    def run(self):

        while True:
            try:
                event = self._events.popleft()
            except IndexError:
                continue

            event.execute_action()


def knock(event: Event) -> None:
    """A callback function that prints the name of the event and sleep for a sec"""
    print(event.name)
    sleep(1)


def who(event: Event) -> None:
    """A callback function that prints the name of the event and sleep for a sec"""
    print(event.name)
    sleep(1)


if __name__ == "__main__":
    event_loop = EventLoop()
    replying = Event("Event 1 for who", who)
    knocking = Event("Event 2 for knock - knock", knock, replying)

    for _ in range(2):
        event_loop.register_event(knocking)
    event_loop.run_forever()
