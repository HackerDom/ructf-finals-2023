FROM node:18.2.0-alpine as build
LABEL stage=builder
WORKDIR /frontend
ENV PATH /frontend/node_modules/.bin:$PATH
COPY ./frontend/package.json /frontend/package.json
RUN npm install
RUN npm install @vue/cli@3.7.0 -g --silent
COPY ./frontend /frontend
RUN npm run build

# production environment
FROM nginx:1.16.0-alpine
COPY --from=build /frontend/dist /usr/share/nginx/html
RUN rm /etc/nginx/conf.d/default.conf
COPY nginx.conf /etc/nginx/conf.d/nginx.conf.template
COPY nginx-entrypoint.sh /
RUN ["chmod", "+x", "/nginx-entrypoint.sh"]
ENTRYPOINT ["/nginx-entrypoint.sh"]
EXPOSE 80
CMD ["nginx", "-g", "daemon off;"]
