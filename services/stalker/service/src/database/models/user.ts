import {
    type BelongsToManyGetAssociationsMixin,
    DataTypes,
    type HasManyCreateAssociationMixin,
    type HasManyRemoveAssociationMixin,
    type Model,
} from 'sequelize';

import connection from '@root/database';

import { type NoteInstance } from './note';

type UserCreationAttributes = {
    name: string;
    password: string;
};

type UserAttributes = Required<UserCreationAttributes> & {
    id: number;
};

type UserInstance = Model<UserAttributes, UserCreationAttributes> & UserAttributes & {
    readonly notes: readonly NoteInstance[];

    createNote: HasManyCreateAssociationMixin<NoteInstance>;
    removeNote: HasManyRemoveAssociationMixin<NoteInstance, NoteInstance['id']>;

    getSharedNotes: BelongsToManyGetAssociationsMixin<NoteInstance>;
};

const User = connection.define<UserInstance>('User', {
    name: {
        type: DataTypes.TEXT,
        allowNull: false,
        unique: true,
    },
    password: {
        type: DataTypes.TEXT,
        allowNull: false,
    },
    id: {
        type: DataTypes.INTEGER,
        primaryKey: true,
        autoIncrement: true,
    },
}, { modelName: 'users' });

export default User;
export type { UserInstance };
