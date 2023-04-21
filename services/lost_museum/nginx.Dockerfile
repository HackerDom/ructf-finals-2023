FROM node:18.2.0-alpine as build
LABEL stage=builder
WORKDIR /front
ENV PATH /front/node_modules/.bin:$PATH
COPY ./front/package.json /front/package.json
RUN npm install
RUN npm install @vue/cli@3.7.0 -g --silent
COPY ./front /front
RUN npm run build

# production environment
FROM nginx:1.16.0-alpine
COPY --from=build /front/dist /usr/share/nginx/html
RUN rm /etc/nginx/conf.d/default.conf
COPY nginx.conf /etc/nginx/conf.d/nginx.conf
EXPOSE 80
CMD ["nginx", "-g", "daemon off;"]