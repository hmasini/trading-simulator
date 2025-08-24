FROM node:20

WORKDIR /app
COPY . .

RUN npm install
RUN npm install react-google-charts
RUN npm run build

EXPOSE 3000

CMD ["npm", "start"]