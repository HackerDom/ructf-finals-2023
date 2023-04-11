import { DataTypes, type ForeignKey, type Model, type ModelAttributes } from 'sequelize';

import connection from '@root/database';

import Note, { type NoteInstance } from './note';
import User, { type UserInstance } from './user';

type ViewersCreationAttributes = ModelAttributes;

type ViewersAttributes = Required<ViewersCreationAttributes> & {
    userId: ForeignKey<UserInstance['id']>;
    noteId: ForeignKey<NoteInstance['id']>;
};

type ViewersInstance = Model<ViewersAttributes, ViewersCreationAttributes> & ViewersAttributes;

const Viewers = connection.define<ViewersInstance>('Viewers', {
    userId: {
        type: DataTypes.INTEGER,
        allowNull: false,
        references: { model: User, key: 'id' },
    },
    noteId: {
        type: DataTypes.INTEGER,
        allowNull: false,
        references: { model: Note, key: 'id' },
    },
}, { modelName: 'viewers' });

export default Viewers;
export type { ViewersInstance };
