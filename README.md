# Central Data Service Playground (CDSP)
The Central Data Service Playground (CDSP) is a neutral, open playground for data services both within and outside the vehicle in the context of data-centric architectures. It enables investigation into the internals of these services and how they can be combined. Furthermore, the playground provides a means to publish and collaborate on such work in the open.

For further information please see the 
<a href="https://covesa.github.io/cdsp/">CDSP documentation</a>.

> [!NOTE]
> CDSP is a young project and there are some gaps to fill here and there in project setup and more comprehensive documentation. Please bear with us as we do that. That said you should find the actual code works. Please ask questions on chat or join the COVESA Data Architecture team call.

## Getting started
A good place to start is the overview in the documentation which introduces why the project exists, the logical concept and its implementation.

Once you are ready to jump into some code the docker deployment readme explains building and starting the playground. Whilst the [examples](https://github.com/COVESA/cdsp/tree/main/examples) folder contains a variety of examples to explore.

### Cloning the source repository
The playground git source repository contains git submodules which need to be initialized either during the clone or afterwords as shown below.

Clone with git v2.13 and later:
```
git clone --recurse-submodules https://github.com/COVESA/cdsp.git
```

Clone with git v1.9 and later:
```
git clone --recursive https://github.com/COVESA/cdsp.git
```

If the repository is already cloned:
```
cd <path to cdsp root>
git submodule update --init --recursive
```

## Known issues
See the github issues for this project with the `bug` label.

## Project resources
The project originated in the COVESA [data architecture team](https://wiki.covesa.global/display/WIK4/Data+Expert+Group%3A+Architecture+and+Infrastructure).

The project maintainers are:
+ Stephen Lawrence, Renesas Electronics
+ Christian Muehlbauer, BMW AG

|Resource|Notes|
|---|---|
|Project management|COVESA Github Project [Central Data Service Playground](https://github.com/orgs/COVESA/projects/8)|
|Issue tracking|Github issues for this project|
|Chat|[Covesa Community Slack channel](https://covesacommunity.slack.com/archives/C06353TRF5F) #data-architecture-pillar|
|Wiki|Data architecture team [project page](https://wiki.covesa.global/display/WIK4/Central+Data+Service+Playground) in COVESA Confluence |