#!/usr/bin/env python3

import dataclasses
from typing import List, Dict, Any


class ProtocolError(Exception):
    pass


class ValidationError(Exception):
    pass


@dataclasses.dataclass
class ServiceError:
    name: str
    message: str | None

    @staticmethod
    def parse(obj: Dict[str, Any]) -> 'ServiceError':
        name = obj.get('name')
        message = obj.get('message')

        assertions = [
            isinstance(name, str),
            any([
                message is None,
                isinstance(message, str),
            ]),
        ]

        if not all(assertions):
            raise ValidationError('invalid error structure')

        return ServiceError(name = name, message = message)


@dataclasses.dataclass
class Note:
    title: str
    visible: bool
    content: str | None
    viewers: List[str] | None
    owner: str

    @staticmethod
    def parse(obj: Dict[str, Any]) -> 'Note':
        title = obj.get('title')
        visible = obj.get('visible')
        content = obj.get('content')
        viewers = obj.get('viewers')
        owner = obj.get('owner')

        assertions = [
            isinstance(title, str),
            isinstance(visible, bool),
            any([
                content is None,
                isinstance(content, str),
            ]),
            any([
                viewers is None,
                all([
                    isinstance(viewers, list),
                    all(isinstance(viewer, str) for viewer in viewers),
                ]),
            ]),
            isinstance(owner, str),
        ]

        if not all(assertions):
            raise ValidationError('invalid note structure')
        
        return Note(
            title = title,
            visible = visible,
            content = content,
            viewers = viewers,
            owner = owner,
        )


@dataclasses.dataclass
class User:
    name: str
    owned_notes: List[str]
    shared_notes: List[str] | None

    @staticmethod
    def parse(obj: Dict[str, Any]) -> 'User':
        name = obj.get('name')
        owned_notes = obj.get('ownedNotes')
        shared_notes = obj.get('sharedNotes')

        assertions = [
            isinstance(name, str),
            all([
                isinstance(owned_notes, list),
                all(isinstance(owned_note, str) for owned_note in owned_notes),
            ]),
            any([
                shared_notes is None,
                all([
                    isinstance(shared_notes, list),
                    all(isinstance(shared_note, str) for shared_note in shared_notes),
                ]),
            ]),
        ]

        if not all(assertions):
            raise ValidationError('invalid user structure')

        return User(
            name = name,
            owned_notes = owned_notes,
            shared_notes = shared_notes,
        )
