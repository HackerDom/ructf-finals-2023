import User, { type UserInstance } from './user';
import Note, { type NoteInstance } from './note';
import Viewers from './viewers';

User.hasMany(Note, { as: 'notes', foreignKey: 'ownerId' });
Note.belongsTo(User, { as: 'owner', foreignKey: 'ownerId' });

User.belongsToMany(Note, {
    as: 'sharedNotes',
    sourceKey: 'id',
    foreignKey: 'userId',
    otherKey: 'noteId',
    through: Viewers,
});
Note.belongsToMany(User, {
    as: 'viewers',
    sourceKey: 'id',
    foreignKey: 'noteId',
    otherKey: 'userId',
    through: Viewers,
});

User.addScope('defaultScope', {
    include: { model: Note.unscoped(), as: 'notes' },
});
Note.addScope('defaultScope', {
    include: { model: User.unscoped(), as: 'owner' },
});

export {
    User,
    type UserInstance,
    Note,
    type NoteInstance,
};
