from locust import HttpLocust, TaskSet;

def index(l):
    l.client.get("/")