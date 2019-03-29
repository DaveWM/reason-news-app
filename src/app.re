[%bs.raw {|require('./app.css')|}];

type article = {
  title: string,
  description: string,
  urlToImage: option(string),
  url: string
};

type response = {
  articles: list(article)
};

type state = {
  searchQuery: option(string),
  articles: list(article)
};

type action = 
  | SearchQueryUpdated(string)
  | ResponseReceived(response);

let initialState = {
  searchQuery: None,
  articles: []
}

let component = ReasonReact.reducerComponent("App");

let isBlank = (s: string) => (s |> String.length) < 3;

let articleDecoder = (json: Js.Json.t): article => 
  Json.Decode.{
    title: json |> field("title", string),
    description: json |> field("description", string),
    urlToImage: json |> optional(field("urlToImage", string)),
    url: json |> field("url", string)
  };

let decodeJson = (json: Js.Json.t): response => 
  Json.Decode.{
    articles: json |> field("articles", list(articleDecoder))
  };

  let toValue = (opt: option('a), default: 'a): 'a => switch opt {
    | None => default
    | Some(x) => x
  };

let make = (_children) => {
  ...component,
  initialState: () => initialState,
  reducer: (action, state) => 
    switch (action) {
    | SearchQueryUpdated(msg) when isBlank(msg) => ReasonReact.Update({...state, searchQuery: None})
    | SearchQueryUpdated(searchQuery) => ReasonReact.UpdateWithSideEffects(
        {...state, searchQuery: Some(searchQuery)}, 
        ({send}) => Fetch.fetch("https://newsapi.org/v2/everything?q=" ++ searchQuery ++ "&apiKey=a35ce68466704851bec15046387412f6") 
          |> Js.Promise.then_(Fetch.Response.json) 
          |> Js.Promise.then_(text => decodeJson(text) ->ResponseReceived |> send |> Js.Promise.resolve)
          |> Js.Promise.catch(_err => {
            Js.log(_err) |> Js.Promise.resolve
          })
          |> ignore
      )
    | ResponseReceived(response) => ReasonReact.Update({...state, articles: response.articles})
    },
  render: ({state, send}) =>
    <div className="App container">
      <div className="App-header">
        <h1>(ReasonReact.string("ReasonML News Search"))</h1>
      </div>
      <div className="App-content">
        <input className="form-control" type_="text" onChange=(ev => (ReactEvent.Form.target(ev)##value) ->SearchQueryUpdated ->send)  placeholder="Type your search query here..."/>
          (ReasonReact.array(Array.of_list(List.map(a => 
          <div className="media article">
            <img src=(a.urlToImage ->toValue("https://via.placeholder.com/80")) className="mr-3 align-self-center" />
            <div className="media-body">
              <h5 className="mt-0">(ReasonReact.string(a.title))</h5>
              <p>(ReasonReact.string(a.description))</p>
              <a href=(a.url)>(ReasonReact.string("Read More..."))</a>
            </div>
          </div>, 
            state.articles))))
      </div>
    </div>
};
