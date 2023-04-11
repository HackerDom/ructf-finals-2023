import {
    type BelongsToManyAddAssociationMixin,
    type BelongsToManyGetAssociationsMixin,
    type BelongsToManyHasAssociationMixin,
    type BelongsToManyRemoveAssociationMixin,
    DataTypes,
    type ForeignKey,
    type Model,
} from 'sequelize';

import connection from '@root/database';

import User, { type UserInstance } from './user';

type NoteCreationAttributes = {
    title: string;
    content: string;
    visible?: boolean;
};

type NoteAttributes = Required<NoteCreationAttributes> & {
    ownerId: ForeignKey<UserInstance['id']>;
    id: number;
};

type NoteInstance = Model<NoteAttributes, NoteCreationAttributes> & NoteAttributes & {
    readonly owner: UserInstance;

    addViewer: BelongsToManyAddAssociationMixin<UserInstance, UserInstance['id']>;
    hasViewer: BelongsToManyHasAssociationMixin<UserInstance, UserInstance['id']>;
    removeViewer: BelongsToManyRemoveAssociationMixin<UserInstance, UserInstance['id']>;
    getViewers: BelongsToManyGetAssociationsMixin<UserInstance>;
};

const Note = connection.define<NoteInstance>('Note', {
    title: {
        type: DataTypes.TEXT,
        allowNull: false,
        unique: true,
    },
    content: {
        type: DataTypes.TEXT,
        allowNull: false,
    },
    visible: {
        type: DataTypes.BOOLEAN,
        primaryKey: true,
        defaultValue: true,
    },
    ownerId: {
        type: DataTypes.INTEGER,
        allowNull: false,
        references: { model: User, key: 'id' },
        onDelete: 'CASCADE',
    },
    id: {
        type: DataTypes.INTEGER,
        primaryKey: true,
        autoIncrement: true,
    },
}, {
    scopes: {
        public: {
            where: { visible: true },
        },
    },
    modelName: 'notes',
});

export default Note;
export type { NoteInstance };
